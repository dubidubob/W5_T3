#pragma once
#include "Splitter.h"

class SSplitterH : public SSplitter
{
public:
	virtual void Drag(FVector2 MouseCoord) override;
	virtual void Render() const override;
};

