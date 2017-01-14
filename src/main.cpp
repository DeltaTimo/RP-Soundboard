// src/main.cpp
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------


#include "common.h"

#include <cstdio>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <cstdarg>
#include <map>

#include <QtWidgets/qmessagebox.h>
#include <QtCore/QString>

#include "main.h"
#include "ts3log.h"
#include "inputfile.h"
#include "samples.h"
#include "config_qt.h"
#include "about_qt.h"
#include "ConfigModel.h"
#include "UpdateChecker.h"
#include "SoundInfo.h"
#include "TalkStateManager.h"


class ModelObserver_Prog : public ConfigModel::Observer
{
public:
	void notify(ConfigModel &model, ConfigModel::notifications_e what, int data) override;
};


static uint64 activeServerId = 1;
static uint64 playingServerId = 1;

ConfigModel *configModel = NULL;
ConfigQt *configDialog = NULL;
AboutQt *aboutDialog = NULL;
Sampler *sampler = NULL;
TalkStateManager *tsMgr = NULL;

ModelObserver_Prog *modelObserver = NULL;
UpdateChecker *updateChecker = NULL;
std::map<uint64, int> connectionStatusMap;
typedef std::lock_guard<std::mutex> Lock;


void ModelObserver_Prog::notify(ConfigModel &model, ConfigModel::notifications_e what, int data)
{
	switch(what)
	{
	case ConfigModel::NOTIFY_SET_VOLUME:
		sampler->setVolume(data);
		break;
	case ConfigModel::NOTIFY_SET_PLAYBACK_LOCAL:
		sampler->setLocalPlayback(model.getPlaybackLocal());
		break;
	case ConfigModel::NOTIFY_SET_MUTE_MYSELF_DURING_PB:
		sampler->setMuteMyself(model.getMuteMyselfDuringPb());
	default:
		break;
	}
}


CAPI void sb_handlePlaybackData(uint64 serverConnectionHandlerID, short* samples, int sampleCount,
	int channels, const unsigned int *channelSpeakerArray, unsigned int *channelFillMask)
{
	if (serverConnectionHandlerID != activeServerId)
		return; //Ignore other servers

	sampler->fetchOutputSamples(samples, sampleCount, channels, channelSpeakerArray, channelFillMask);
}


CAPI void sb_handleCaptureData(uint64 serverConnectionHandlerID, short* samples, int sampleCount, int channels, int* edited)
{
	if (serverConnectionHandlerID != activeServerId)
		return; //Ignore other servers

	int written = sampler->fetchInputSamples(samples, sampleCount, channels, NULL);
	if(written > 0)
		*edited |= 0x1;
}


int sb_playFile(const SoundInfo &sound)
{
	if (activeServerId == 0)
		return 2;
	return sampler->playFile(sound) ? 0 : 1;
}


Sampler *sb_getSampler()
{
	return sampler;
}


void sb_enableInterface(bool enabled) 
{
	configDialog->setEnabled(enabled);
}

CAPI void sb_init()
{
#ifdef _DEBUG
	QMessageBox::information(NULL, "", "rp soundboard plugin init, attach debugger now");
#endif

	InitFFmpegLibrary();
	
	configModel = new ConfigModel();
	configModel->readConfig();

	sampler = new Sampler();
	sampler->init();

	tsMgr = new TalkStateManager();
	QObject::connect(sampler, &Sampler::onStartPlaying,   tsMgr, &TalkStateManager::onStartPlaying, Qt::QueuedConnection);
	QObject::connect(sampler, &Sampler::onStopPlaying,    tsMgr, &TalkStateManager::onStopPlaying, Qt::QueuedConnection);
	QObject::connect(sampler, &Sampler::onPausePlaying,   tsMgr, &TalkStateManager::onPauseSound, Qt::QueuedConnection);
	QObject::connect(sampler, &Sampler::onUnpausePlaying, tsMgr, &TalkStateManager::onUnpauseSound, Qt::QueuedConnection);

	configDialog = new ConfigQt(configModel);
	configDialog->showMinimized();
	configDialog->hide();

	modelObserver = new ModelObserver_Prog();
	configModel->addObserver(modelObserver);

	configModel->notifyAllEvents();

	updateChecker = new UpdateChecker();
	updateChecker->startCheck();
}


CAPI void sb_saveConfig()
{
	configModel->writeConfig();
}


CAPI void sb_kill()
{
	configModel->remObserver(modelObserver);
	delete modelObserver; 
	modelObserver = NULL;

	sampler->shutdown();
	delete sampler;
	sampler = NULL;

	configDialog->close();
	delete configDialog;
	configDialog = NULL;

	configModel->writeConfig();
	delete configModel;
	configModel = NULL;

	if(aboutDialog)
	{
		aboutDialog->close();
		delete aboutDialog;
		aboutDialog = NULL;
	}

	delete updateChecker;
	updateChecker = NULL;
}


CAPI void sb_onServerChange(uint64 serverID)
{
	if (connectionStatusMap.find(serverID) == connectionStatusMap.end())
		connectionStatusMap[serverID] = STATUS_DISCONNECTED;
	bool connected = connectionStatusMap[serverID] == STATUS_CONNECTION_ESTABLISHED;

	tsMgr->setActiveServerId(serverID);
	activeServerId = serverID;
	logInfo("Server Id: %ull", (unsigned long long)serverID);
	sb_enableInterface(connected);
}


CAPI void sb_openDialog()
{
	if(!configDialog)
		configDialog = new ConfigQt(configModel);
	configDialog->showNormal();
	configDialog->raise();
	configDialog->activateWindow();

	if (connectionStatusMap[activeServerId] != STATUS_CONNECTION_ESTABLISHED)
		QMessageBox::information(configDialog, "No server connection",
		"You are not connected to a server.\n"
		"RP Soundboard is disabled until you are connected properly.");
}


CAPI void sb_stopPlayback()
{
	sampler->stopPlayback();
}


CAPI void sb_pauseSound()
{
	sampler->pausePlayback();
}


CAPI void sb_unpauseSound()
{
	sampler->unpausePlayback();
}


CAPI void sb_pauseButtonPressed()
{
	if (sampler->getState() == Sampler::ePLAYING)
		sb_pauseSound();
	else if (sampler->getState() == Sampler::ePAUSED)
		sb_unpauseSound();
}


CAPI void sb_playButton(int btn)
{
	const SoundInfo *sound = configModel->getSoundInfo(btn);
	if(sound)
		sb_playFile(*sound);
}


CAPI void sb_openAbout()
{
	if(!aboutDialog)
		aboutDialog = new AboutQt();
	aboutDialog->show();
}


CAPI void sb_onConnectStatusChange(uint64 serverConnectionHandlerID, int newStatus, unsigned int errorNumber) 
{
	if(newStatus == STATUS_DISCONNECTED)
		connectionStatusMap.erase(serverConnectionHandlerID);
	else
		connectionStatusMap[serverConnectionHandlerID] = newStatus;

	if (serverConnectionHandlerID == activeServerId)
	{
		if (newStatus == STATUS_DISCONNECTED)
			sb_stopPlayback();
		sb_enableInterface(newStatus == STATUS_CONNECTION_ESTABLISHED);
	}
}


CAPI void sb_getInternalHotkeyName(int buttonId, char *buf)
{
	sprintf(buf, "button_%i", buttonId + 1);
}


CAPI void sb_onHotkeyRecordedEvent(const char *keyword, const char *key)
{
	if (configDialog)
		configDialog->onHotkeyRecordedEvent(keyword, key);
}


CAPI void sb_onStopTalking()
{
	tsMgr->onClientStopsTalking();
}

