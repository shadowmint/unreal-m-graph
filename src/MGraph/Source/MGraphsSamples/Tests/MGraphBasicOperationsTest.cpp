#include "MockGraphNode.h"
#include "MGraphs/MGraph.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MGraphBasicOperationsTest, "Tests.MGraphBasicOperationsTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MGraphBasicOperationsTest::RunTest(const FString& Parameters)
{
	auto const Config = MockGraphNode::MakeGraphConfig();
	auto Graph = TMGraph<MockGraphNode>::Make(Config);

	auto const A = Graph->Add(MockGraphNode(FVector(0, 0, 0)));
	auto const B = Graph->Add(MockGraphNode(FVector(10, 0, 0)));
	auto const C = Graph->Add(MockGraphNode(FVector(0, 10, 0)));
	auto const D = Graph->Add(MockGraphNode(FVector(0, 0, 10)));

	Graph->Connect(A, B);
	Graph->Connect(B, C);
	Graph->Connect(C, D);
	Graph->Connect(D, A);

	TArray<FMGraphEdge<MockGraphNode>*> Edges;
	check(A->Edges.Num() == 1);
	check(A->Id == 0);
	check(B->Id == 1);
	check(C->Id == 2);
	check(D->Id == 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MGraphBasicOperationsTest_Large, "Tests.MGraphBasicOperationsTest_Large",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MGraphBasicOperationsTest_Large::RunTest(const FString& Parameters)
{
	auto const Config = MockGraphNode::MakeGraphConfig();
	auto Graph = TMGraph<MockGraphNode>::Make(Config);

	TArray<FMGraphVertex<MockGraphNode> *> Nodes;
	for (auto I = 0; I < 1000; I++)
	{
		auto Node = Graph->Add(MockGraphNode(FVector(I % 3, I % 3 + 1, I % 3 + 2)));
		Nodes.Add(Node);
	}

	for (auto I = 0; I < 1000; I++)
	{
		check(Nodes[I]->Id == I);
	}
	
	return true;
}
