// Copyright@TXLib All rights reserved.
// Author: TX Studio: TX_Jerry
// Module: TXGraphics
// File: render_engine.h

#pragma once
#include "glad/glad.h"
#include "tx/math.h"
#include <array>

using namespace tx;

class Learn {
public:
	void init() {
		glGenBuffers(1, &bufferId);
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), vertexBufferData.data(), GL_STATIC_DRAW);
	}
	void update() {
	}
	void render() {
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

private:
	u32 bufferId;
	// clang-format off
	std::array<float, 6> vertexBufferData = {
		-0.5f, -0.5f,
		 0.5f, -0.5f,
		 0.0f,  0.5f
	};
	// clang-format on
};