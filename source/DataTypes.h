#pragma once

enum class CullMode 
{ 
	BackFaceCulling,
	FrontFaceCulling, 
	NoCulling 
};

enum class RenderMode
{
	Combined,
	ObservedArea,
	Diffuse,
	Specular
};