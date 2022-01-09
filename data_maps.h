/*
 * homm3-wallpaper, live HOMM3 wallpaper
 * Copyright (C) 2022 i.Dark_Templar <darktemplar@dark-templar-archives.net>
 *
 * Subject to terms and condition provided in LICENSE.txt
 *
 */

#pragma once

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "vcmi/GameConstants.h"

extern const std::vector<ArtifactID> treasure_artifacts;
extern const std::vector<ArtifactID> minor_artifacts;
extern const std::vector<ArtifactID> major_artifacts;
extern const std::vector<ArtifactID> relic_artifacts;
extern const std::vector<ArtifactID> combination_artifacts;

extern const std::map<ETownType, std::vector<std::string> > dwellings_map;
extern const std::vector<std::vector<std::string> > monsters_map;

extern const std::map<ETownType, std::vector<std::string> > towns_map;

extern const std::vector<std::string> hero_subtype_appearance_map;
extern const std::map<ETownType, std::vector<std::string> > hero_by_town_map;

extern const std::vector<std::string> resources_map;

extern const std::vector<std::pair<std::string, int> > hero_flags_map;
