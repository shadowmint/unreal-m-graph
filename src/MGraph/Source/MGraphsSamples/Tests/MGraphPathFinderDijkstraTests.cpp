#include "MockGraphNode.h"
#include "MGraphs/PathFinder/MGraphPathFinderDijkstra.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MGraphPathFinderDijkstraTests_SimpleUniDirectional, "Tests.MGraphPathFinderDijkstraTests_SimpleUniDirectional",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MGraphPathFinderDijkstraTests_SimpleUniDirectional::RunTest(const FString& Parameters)
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

	auto const PathFinder = MakeUnique<TMGraphPathFinderDijkstra<MockGraphNode>>();
	TArray<FMGraphEdge<MockGraphNode>*> Path;

	// Simple path, A -> B, single step
	auto FoundPath = PathFinder->FindPath(A, B, *Graph, Path);
	check(FoundPath);
	check(Path.Num() == 1);
	check(Path[0]->FromVertex == A);
	check(Path[0]->ToVertex == B);

	// Linear path, A -> B -> C -> D, no backtracking allowed (ie. A -> D is not linked)
	FoundPath = PathFinder->FindPath(A, D, *Graph, Path);
	check(FoundPath);
	check(Path.Num() == 3);
	check(Path[0]->FromVertex == A);
	check(Path[0]->ToVertex == B);
	check(Path[1]->FromVertex == B);
	check(Path[1]->ToVertex == C);
	check(Path[2]->FromVertex == C);
	check(Path[2]->ToVertex == D);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MGraphPathFinderDijkstraTests_SimpleBiDirectional, "Tests.MGraphPathFinderDijkstraTests_SimpleBiDirectional",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MGraphPathFinderDijkstraTests_SimpleBiDirectional::RunTest(const FString& Parameters)
{
	auto const Config = MockGraphNode::MakeGraphConfig();
	auto Graph = TMGraph<MockGraphNode>::Make(Config);

	auto const A = Graph->Add(MockGraphNode(FVector(0, 0, 0)));
	auto const B = Graph->Add(MockGraphNode(FVector(10, 0, 0)));
	auto const C = Graph->Add(MockGraphNode(FVector(0, 10, 0)));
	auto const D = Graph->Add(MockGraphNode(FVector(0, 0, 10)));

	Graph->Connect(A, B, true);
	Graph->Connect(B, C, true);
	Graph->Connect(C, D, true);
	Graph->Connect(D, A, true);

	auto const PathFinder = MakeUnique<TMGraphPathFinderDijkstra<MockGraphNode>>();
	TArray<FMGraphEdge<MockGraphNode>*> Path;

	// Simple path, A -> B, single step
	auto FoundPath = PathFinder->FindPath(A, B, *Graph, Path);
	check(FoundPath);
	check(Path.Num() == 1);
	check(Path[0]->FromVertex == A);
	check(Path[0]->ToVertex == B);

	// Linear path, A -> B -> C -> D, no backtracking allowed (ie. A -> D is not linked)
	FoundPath = PathFinder->FindPath(A, D, *Graph, Path);
	check(FoundPath);
	check(Path.Num() == 1);
	check(Path[0]->FromVertex == A);
	check(Path[0]->ToVertex == D);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(MGraphPathFinderDijkstraTests_ComplexPath, "Tests.MGraphPathFinderDijkstraTests_ComplexPath",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool MGraphPathFinderDijkstraTests_ComplexPath::RunTest(const FString& Parameters)
{
	auto const Config = MockGraphNode::MakeGraphConfig();
	auto Graph = TMGraph<MockGraphNode>::Make(Config);

	auto const N0 = Graph->Add(MockGraphNode(FVector(0, 0, 0)));
	auto const N1 = Graph->Add(MockGraphNode(FVector(10, 0, 0)));
	auto const N2 = Graph->Add(MockGraphNode(FVector(20, 0, 0)));
	auto const N3 = Graph->Add(MockGraphNode(FVector(30, 0, 0)));
	auto const N4 = Graph->Add(MockGraphNode(FVector(20, 0, 0)));
	auto const N5 = Graph->Add(MockGraphNode(FVector(20, 10, 0)));
	auto const N6 = Graph->Add(MockGraphNode(FVector(20, 20, 0)));
	auto const N7 = Graph->Add(MockGraphNode(FVector(20, 30, 0)));

	// X
	Graph->Connect(N0, N1, true);
	Graph->Connect(N1, N2, true);
	Graph->Connect(N2, N3, true);

	// Y
	Graph->Connect(N4, N5, true);
	Graph->Connect(N5, N6, true);
	Graph->Connect(N6, N7, true);

	// Join links
	Graph->Connect(N2, N4, true); // Shorter path
	Graph->Connect(N3, N4, true); // Longer path

	auto const PathFinder = MakeUnique<TMGraphPathFinderDijkstra<MockGraphNode>>();
	TArray<FMGraphEdge<MockGraphNode>*> Path;

	// Resolve linear path
	auto FoundPath = PathFinder->FindPath(N0, N3, *Graph, Path);
	check(FoundPath);
	check(Path.Num() == 3);
	
	// Resolve complex path
	auto FoundPath2 = PathFinder->FindPath(N0, N7, *Graph, Path);
	check(FoundPath2);

	for (auto Link : Path)
	{
		UE_LOG(LogTemp, Display, TEXT("Found: %d -> %d"), Link->FromVertex->Id, Link->ToVertex->Id);
	}
	check(Path.Num() == 6);
	
	return true;
}
