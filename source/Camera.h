#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };

		float nearPlane{ 0.1f }, farPlane{ 100.f };

		float aspectRatio;

		bool hasMoved{ true };
		bool hasChangedFov{ true };

		Vector3 forward{Vector3::UnitZ};
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		void Initialize(float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f }, float _aspectRatio = 16.f / 9.f)
		{
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			aspectRatio = _aspectRatio;

			origin = _origin;
		}

		void Update(const Timer* pTimer)
		{
			const float deltaTime{ pTimer->GetElapsed() };

			//Camera Update Logic
			float movementSpeed{ 5.f };
			float rotationSpeed{ 1/300.f };
			
			//Keyboard
			HandleKeyboardInput(movementSpeed, rotationSpeed, deltaTime);

			//Mouse
			constexpr float factor{ 0.2f };
			HandleMouseInput(factor * movementSpeed, rotationSpeed, deltaTime);

			//Update Matrices
			if (hasMoved)
			{
				CalculateViewMatrix();
				hasMoved = false;
			}

			if (hasChangedFov)
			{
				fov = tanf((fovAngle * TO_RADIANS) / 2.f);

				CalculateProjectionMatrix();
				hasChangedFov = false;
			}
		}

	private:
		void CalculateViewMatrix()
		{
			//viewMatrix = inverse cameraToWorld
			viewMatrix = Matrix::CreateLookAtLH(origin, forward, Vector3::UnitY);

			//invViewMatrix = cameraToWorld			
			invViewMatrix = Matrix::Inverse(viewMatrix);

			right = invViewMatrix.GetAxisX();
			up = invViewMatrix.GetAxisY();

			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh
		}

		void CalculateProjectionMatrix()
		{
			projectionMatrix = Matrix::CreatePerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane);

			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void HandleKeyboardInput(float& movementSpeed, float& rotationSpeed, float deltaTime)
		{

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				constexpr float factor{ 4.f };

				movementSpeed *= factor;
				rotationSpeed *= factor;
			}

			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
			{
				origin += forward * movementSpeed * deltaTime;
				hasMoved = true;
			}

			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin -= forward * movementSpeed * deltaTime;
				hasMoved = true;
			}

			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin += right * movementSpeed * deltaTime;
				hasMoved = true;
			}

			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin -= right * movementSpeed * deltaTime;
				hasMoved = true;
			}

			if (pKeyboardState[SDL_SCANCODE_E])
			{
				fovAngle -= movementSpeed * deltaTime;
				fovAngle = std::max(fovAngle, 1.f);

				hasChangedFov = true;
			}

			if (pKeyboardState[SDL_SCANCODE_Q])
			{
				fovAngle += movementSpeed * deltaTime;
				fovAngle = std::min(fovAngle, 179.f);

				hasChangedFov = true;
			}
		}

		void HandleMouseInput(float movementSpeed, float rotationSpeed, float deltaTime)
		{
			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);
			
			const float directionX{ static_cast<float>(mouseX) }, directionY{ static_cast<float>(mouseY) };

			//Error fix when framerate is to high
			movementSpeed *= std::max(deltaTime, 1.f / 30.f);

			//Left
			if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				//Left + Right
				if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
				{
					origin -= up * directionY * movementSpeed;
				}
				else
				{
					origin += right * directionX * movementSpeed;
					origin -= forward * directionY * movementSpeed;
				}

				hasMoved = true;
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				forward = Matrix::CreateRotationY(directionX * rotationSpeed).TransformVector(forward);
				forward = Matrix::CreateRotationX(-directionY * rotationSpeed).TransformVector(forward);

				hasMoved = true;
			}
		}
	};
}
