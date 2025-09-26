cbuffer PickCB : register(b2)
{
	uint Pick;
	uint ObjectID;
	int2 Padding;
}

uint mainPS() : SV_Target
{
	return ObjectID;
}
