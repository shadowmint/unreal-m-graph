#pragma once
#include "MGraphs/MGraph.h"

class MockGraphNode
{
public:
	FVector Position;

	MockGraphNode(FVector InPosition)
	{
		Position = InPosition;
	}

	static float GetCost(MockGraphNode* A, MockGraphNode* B)
	{
		if (A == B) return 0;
		return (A->Position - B->Position).Length();
	}

	static FMGraphConfig<MockGraphNode> MakeGraphConfig()
	{
		FMGraphConfig<MockGraphNode> Config;
		Config.GetCost = GetCost;

		return Config;
	}
};
