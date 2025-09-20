#pragma once
#include "Slate/Splitter.h"

class SSplitterV : public SSplitter
{
public:
	virtual void Drag(FVector2 MouseCoord) override;
};
