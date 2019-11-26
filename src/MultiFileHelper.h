#pragma once
#ifndef rpsbsrc__multifilehelper_H__
#define rpsbsrc__multifilehelper_H__

#include <string>
#include <vector>

namespace MultiFileHelper {
	extern inline bool IsMultipleFiles(const std::string& str) {
		return str.find(";") != std::string::npos;
	}

	extern inline std::vector<std::string> ListFromDelimited(std::string delimitedString) {
		std::string fileName;
		std::vector<std::string> fileNames;
		std::size_t current, previous = 0;
		while ((current = delimitedString.find(";")) != std::string::npos) {
			fileNames.push_back(delimitedString.substr(0, current));
			delimitedString.erase(0, current + 1);
		}
		fileNames.push_back(delimitedString);

		return fileNames;
	}

	extern inline std::vector<QString> QListFromDelimited(QString delimited) {
		std::vector<std::string> fileNames = ListFromDelimited(delimited.toStdString());
		std::vector<QString> qFileNames;
		for each (std::string filename in fileNames) {
			qFileNames.push_back(QString(filename.c_str()));
		}
		return qFileNames;
	}

	extern inline std::string DelimitedFromList(std::vector<std::string> fileNames) {
		std::string str;
		size_t i = 0;
		size_t size = fileNames.size();
		for each (std::string filename in fileNames) {
			str += filename;
			if (i != size - 1 && size > 1) {
				str += ";";
			}
			++i;
		}
		return str;
	}

	extern inline QString QDelimitedFromList(std::vector<QString> fileNames) {
		std::string str;
		size_t i = 0;
		size_t size = fileNames.size();
		for each (QString filename in fileNames) {
			str += filename.toStdString();
			if (i != size - 1 && size > 1) {
				str += ";";
			}
			++i;
		}
		return QString(str.c_str());
	}

	extern inline std::string RandomSoundFromDelimited(std::string delimitedString) {
		std::string fileName;
		std::vector<std::string> fileNames;
		std::size_t current, previous = 0;
		while ((current = delimitedString.find(";")) != std::string::npos) {
			fileNames.push_back(delimitedString.substr(0, current));
			delimitedString.erase(0, current + 1);
		}
		fileNames.push_back(delimitedString);
		fileName = fileNames[std::rand() % fileNames.size()];

		return fileName;
	}
}

#endif