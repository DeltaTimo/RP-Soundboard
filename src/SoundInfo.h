// src/SoundInfo.h
//----------------------------------
// RP Soundboard Source Code
// Copyright (c) 2015 Marius Graefe
// All rights reserved
// Contact: rp_soundboard@mgraefe.de
//----------------------------------

#pragma once
#ifndef rpsbsrc__SoundInfo_H__
#define rpsbsrc__SoundInfo_H__

#include <QtCore/QSettings>
#include <QColor>
#include "MultiFileHelper.h"

class SoundInfo
{
public:
	SoundInfo();
	void readFromConfig(const QSettings &settings);
	void saveToConfig(QSettings &settings) const;
	double getStartTime() const;
	double getPlayTime() const;

	static double getTimeUnitFactor(int unit);
	bool customColorEnabled() const { return customColor.alpha() != 0; }
	void setCustomColorEnabled(bool enabled) { customColor.setAlpha(enabled ? 255 : 0); }
	QString getFilename(size_t i) const { return filenames.size() >= i ? filenames.at(i) : getFilename(); }
	QString getFilename() const { return filenames.size() >= 1 ? filenames.at(0) : QString(); }
	std::vector<QString> getListFromDelimited(QString delimited) const { return MultiFileHelper::QListFromDelimited(delimited); }
	void setFilename(QString fileNames) {
		filenames.clear();
		for each (QString filename in MultiFileHelper::QListFromDelimited(fileNames)) {
			if (!filename.isEmpty()) filenames.push_back(filename);
		}
	}
	inline QString randomSound() const { return filenames[std::rand() % filenames.size()]; }
	inline bool isMultiple() const { return filenames.size() > 1; }
	inline bool isEmpty() const { return filenames.size() == 0 || getFilename().isEmpty(); }

public:
	// QString filename;
	std::vector<QString> filenames;
	QString customText;
	QColor customColor;
	int volume;
	bool cropEnabled;
	int cropStartValue;
	int cropStartUnit;
	int cropStopAfterAt;
	int cropStopValue;
	int cropStopUnit;
};

#endif // rpsbsrc__SoundInfo_H__
