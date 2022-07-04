// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MGraphs/MGraph.h"

// Turn this on for verbose logging
#define TMGraphPathFinderDijkstra_DEBUG 0

template <typename T>
struct TMGraphPathFinderDijkstraRecord
{
	FMGraphVertex<T>* Node = nullptr;
	TMGraphPathFinderDijkstraRecord<T>* Prev = nullptr;
	FMGraphEdge<T>* Edge = nullptr;
	float CostSoFar;
};

template <typename T>
class TMGraphPathFinderDijkstra
{
public:
	/** Public API */
	bool FindPath(FMGraphVertex<T>* FromNode, FMGraphVertex<T>* ToNode, TMGraph<T>& OverGraph, TArray<FMGraphEdge<T>*>& Path)
	{
		// Setup data
		Records.Reset();
		Path.Reset();

#if TMGraphPathFinderDijkstra_DEBUG
		UE_LOG(LogTemp, Display, TEXT("Starting path query %d -> %d"), FromNode->Id, ToNode->Id);
#endif

		// Check zero-length path
		if (OverGraph.AreEqual(FromNode, ToNode))
		{
			return true;
		}

		// Setup the start record
		auto StartRecord = MakeRecord(FromNode, nullptr);

		// Create the lists we'll use
		TArray<TMGraphPathFinderDijkstraRecord<T>*> Open;
		TArray<TMGraphPathFinderDijkstraRecord<T>*> Closed;
		Open.Add(StartRecord);

		// Process all open nodes
		TMGraphPathFinderDijkstraRecord<T>* Current = nullptr;
		while (Open.Num() > 0)
		{
			Current = FindSmallestElementIn(Open);
#if TMGraphPathFinderDijkstra_DEBUG
			UE_LOG(LogTemp, Display, TEXT("Next open node %d, remaining: %d"), Current->Node->Id, Open.Num());
#endif
			// If the node we just looked at is the goal, end
			if (OverGraph.AreEqual(Current->Node, ToNode))
			{
#if TMGraphPathFinderDijkstra_DEBUG
				UE_LOG(LogTemp, Display, TEXT("Found end"));
#endif

				break;
			}

			// Otherwise, lets find all connections for this node
#if TMGraphPathFinderDijkstra_DEBUG
			UE_LOG(LogTemp, Display, TEXT("Processing: %d edges on node %d"), Current->Node->Edges.Num(), Current->Node->Id);
#endif

			for (auto& Connection : Current->Node->Edges)
			{
#if TMGraphPathFinderDijkstra_DEBUG
				UE_LOG(LogTemp, Display, TEXT("Processing: Edges %d -> %d"), Connection.FromVertex->Id, Connection.ToVertex->Id);
#endif

				auto EndNodeCost = Current->CostSoFar + OverGraph.GetCost(&Connection);
				auto EndNodeRecord = GetRecord(Connection.ToVertex, Open, OverGraph);
				auto OpenContainsRecord = EndNodeRecord != nullptr;

				// If the node is already in the closed list, continue
				if (ContainsRecord(Connection.ToVertex, Closed, OverGraph))
				{
#if TMGraphPathFinderDijkstra_DEBUG
					UE_LOG(LogTemp, Display, TEXT("Target node %d is already in closed"), Connection.ToVertex->Id);
#endif

					continue;
				}

				// If this is an open record, check if this route is worse.
				// If this route is worse than the open node, drop this one.
				if (OpenContainsRecord && EndNodeRecord->CostSoFar <= EndNodeCost)
				{
#if TMGraphPathFinderDijkstra_DEBUG
					UE_LOG(LogTemp, Display, TEXT("Skip! The cost to reach here (%f) > best (%f)"), EndNodeCost, EndNodeRecord->CostSoFar);
#endif

					continue;
				}

				// If we got here we know we have an unvisited node, so make a record
				if (!EndNodeRecord)
				{
#if TMGraphPathFinderDijkstra_DEBUG
					UE_LOG(LogTemp, Display, TEXT("New record for target %d"), Connection.ToVertex->Id);
#endif

					EndNodeRecord = MakeRecord(Connection.ToVertex, Current);
				}

				// If we got here, this is a better cost than our previous record, or it's a new record
				EndNodeRecord->CostSoFar = EndNodeCost;
				EndNodeRecord->Edge = &Connection;
#if TMGraphPathFinderDijkstra_DEBUG
				UE_LOG(LogTemp, Display, TEXT("Set the cost to -> %d to %f"), Connection.ToVertex->Id, EndNodeCost);
#endif

				// Save it to the open list if it's not already a member
				if (!OpenContainsRecord)
				{
#if TMGraphPathFinderDijkstra_DEBUG
					UE_LOG(LogTemp, Display, TEXT("Added record to open list"));
#endif

					Open.Add(EndNodeRecord);
				}
			}

			// We've finished processing this node, so remove it from open and add it to closed
			Closed.Add(Current);
			Open.Remove(Current);
#if TMGraphPathFinderDijkstra_DEBUG
			UE_LOG(LogTemp, Display, TEXT("Completed processing for node %d"), Current->Node->Id);
#endif
		}

		// If we got here we either found the goal or we ran out of paths to explore.
		if (Current == nullptr || !OverGraph.AreEqual(Current->Node, ToNode))
		{
#if TMGraphPathFinderDijkstra_DEBUG
			if (Current == nullptr)
			{
				UE_LOG(LogTemp, Display, TEXT("Invalid end; the current node was NULL (no elements in graph)"));
			}
			else
			{
				UE_LOG(LogTemp, Display, TEXT("Invalid end; the final node we checked (%d) was not the target (%d)"), Current->Node->Id, ToNode->Id);
			}
#endif
			return false;
		}

		// If we did find a path, return along the path and collect it
		Path.Reset();
		while (Current->Node != FromNode)
		{
			Path.Insert(Current->Edge, 0);
			Current = Current->Prev;
		}

#if TMGraphPathFinderDijkstra_DEBUG
		UE_LOG(LogTemp, Display, TEXT("Done"));
#endif

		return true;
	}

private:
	/** Actual records */
	TArray<TUniqueObj<TMGraphPathFinderDijkstraRecord<T>>> Records;

	/** Create a new record and return a reference to it */
	TMGraphPathFinderDijkstraRecord<T>* MakeRecord(FMGraphVertex<T>* Node, TMGraphPathFinderDijkstraRecord<T>* Parent)
	{
		if (Node == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("TMGraphPathFinderDijkstraRecord::MakeRecord: Invalid node in create record"));
			return nullptr;
		}

		auto Record = TMGraphPathFinderDijkstraRecord<T>();
		Record.Node = Node;
		Record.Prev = Parent;
		Record.Edge = nullptr;
		Record.CostSoFar = 0;

		Records.Add(TUniqueObj<TMGraphPathFinderDijkstraRecord<T>>(Record));
		return &Records[Records.Num() - 1].Get();
	}

	TMGraphPathFinderDijkstraRecord<T>* FindSmallestElementIn(TArray<TMGraphPathFinderDijkstraRecord<T>*>& Open)
	{
		auto Min = Open[0];
		for (auto const V : Open)
		{
			if (V->CostSoFar < Min->CostSoFar)
			{
				Min = V;
			}
		}
		return Min;
	}

	TMGraphPathFinderDijkstraRecord<T>* GetRecord(FMGraphVertex<T>* Node, TArray<TMGraphPathFinderDijkstraRecord<T>*>& InArray, TMGraph<T>& Graph)
	{
		for (auto const Value : InArray)
		{
			if (Graph.AreEqual(Value->Node, Node))
			{
				return Value;
			}
		}
		return nullptr;
	}

	bool ContainsRecord(FMGraphVertex<T>* Node, TArray<TMGraphPathFinderDijkstraRecord<T>*>& InArray, TMGraph<T>& Graph)
	{
		return GetRecord(Node, InArray, Graph) != nullptr;
	}
};
