#pragma once
#include <iostream>
#include <stdio.h>

char* read_file(const char* filename)
{
	FILE* f;
	fopen_s(&f, filename, "rb");
	if (f == NULL)
		return NULL;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	rewind(f);
	char* bfr = (char*)malloc(sizeof(char) * (size + 1));
	if (bfr == NULL)
		return NULL;
	long ret = fread(bfr, 1, size, f);
	if (ret != size)
		return NULL;
	bfr[size] = '\0';
	return bfr;
}

glm::vec3 vectorToVec3(const std::vector<float>& vec) {
	if (vec.size() < 3) {
		throw std::invalid_argument("Vector must have at least three elements.");
	}
	return glm::vec3(vec[0], vec[1], vec[2]);
}