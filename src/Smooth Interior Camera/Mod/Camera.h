/*
*********************************************************************
* File			: Camera.h
* Project		: Smooth Interior Camera
* Developers	: Piotr Krupa (piotrkrupa06@gmail.com)
* License		: MIT License
*********************************************************************
*/

#pragma once

#include "Game\prism.h"

class Camera
{
	enum MoveTo
	{
		BIGGER,
		SMALLER
	};		

private:
	prism::Camera	*m_pGameCamera = nullptr;

	MoveTo			m_moveType;

	float			m_rxCurr = 0.f;
	float			m_rxEnd = 0.f;

	float			m_delta;
	int				m_dir;

	float			m_acceleration;
	float			m_speed = 0.01f;

	int				m_movingStage;

	bool			m_anim = false;
	bool			m_stopInNextPulse = false;

	const float MAX_LINEARLY_SPEED = 0.12f;
	const float MAX_SMOOTH_SPEED = 0.01f;

public:
	Camera();
	~Camera();

	void MoveTo(float rx);
	void UpdateRX(float rx) { m_rxCurr = rx; }

	void UpdateGameCamera(prism::Camera *gameCamera) { m_pGameCamera = gameCamera; }

	prism::Camera *GetGameCamera() { return m_pGameCamera; }

	void Pulse();

	bool IsAnimation() { return m_anim; }
};
