#pragma once
#ifndef rpsbsrc__multifilehelper_H__
#define rpsbsrc__multifilehelper_H__

#include <string>
#include <vector>

namespace MultiFileHelper {

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
		printf(fileName.c_str()); // TODO: Remove

		return fileName;
	}
}

#endif