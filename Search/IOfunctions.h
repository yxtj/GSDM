#pragma once
#include "Option.h"
#include "Subject.h"
#include <map>
#include <iostream>
#include <string>

//---------------------------- Time Course ---------------------------------


//---------------------------- Correlation ---------------------------------

// correlation file name functions:
// filename format: <type>-<subject Id>-<scan Id>.txt
std::string genCorrFilename(const Subject& sub);
bool checkCorrFilename(const std::string& fn);
Subject parseCorrFilename(const std::string& fn) noexcept(false);
bool checknParseCorrFilename(const std::string& fn, Subject* pRes) noexcept;

//---------------------------- Graph ---------------------------------

// graph file name functions:
// filename format: <type>-<subject Id>-<scan Id>.txt
std::string genGraphFilename(const Subject& sub);
bool checknParseGraphFilename(const std::string& fn, Subject* pRes) noexcept;


