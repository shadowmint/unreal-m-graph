// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

template <typename T>
struct FMGraphConfig
{
	/** Resolve the actual cost for traversal between the connected nodes A and B */
	TFunction<float(T* A, T* B)> GetCost;
};

template <typename T>
struct FMGraphVertex;

template <typename T>
struct FMGraphEdge
{
	/** Index into the graphs VertexData for the start of the edge */
	FMGraphVertex<T>* FromVertex;

	/** Index into the graphs VertexData for the end of the edge */
	FMGraphVertex<T>* ToVertex;

	/** The traversal cost to go from -> to */
	TOptional<float> Cost;
};

template <typename T>
struct FMGraphVertex
{
	int Id;

	T VertexData;

	/** Set of edges with this as the source vertex */
	TArray<FMGraphEdge<T>> Edges;
};

template <typename T>
class TMGraph
{
private:
	/* Internal data =========================================*/

	/** Id offset */
	int NextId = 0;

	/** Type binding */
	FMGraphConfig<T> Config;

	/** Internal vertex list */
	TArray<TUniqueObj<FMGraphVertex<T>>> VertexData;

public:
	/** Create a new empty graph */
	static TUniquePtr<TMGraph<T>> Make(FMGraphConfig<T> InConfig)
	{
		auto Instance = MakeUnique<TMGraph<T>>();
		(*Instance).Config = InConfig;
		(*Instance).Reset();
		return Instance;
	}

	/** Add a vertex */
	FMGraphVertex<T>* Add(T Data)
	{
		auto Node = TUniqueObj<FMGraphVertex<T>>(FMGraphVertex<T>{
			NextId,
			Data,
			TArray<FMGraphEdge<T>>()
		});
		VertexData.Add(Node);
		NextId += 1;
		return &VertexData[VertexData.Num() - 1].Get();
	}

	/**
	 * Connect a vertex to another vertex.
	 * If `CreateReverseLink` is true, automatically create the reverse link too.
	 */
	void Connect(FMGraphVertex<T>* From, FMGraphVertex<T>* To, bool Bidirectional = false)
	{
		if (!From || !To) return;
		if (!Connected(From, To))
		{
			From->Edges.Add(FMGraphEdge<T>{
				From,
				To,
				TOptional<float>()
			});
		}
		if (Bidirectional)
		{
			if (!Connected(To, From))
			{
				To->Edges.Add(FMGraphEdge<T>{
					To, // Reverse direction
					From,
					TOptional<float>()
				});
			}
		}
	}

	/** 
	 * If Bidirectional is false, returns true if From connects to To
	 * If Bidirectional is true, returns true if BOTH From -> To and To -> From.
	 */
	bool Connected(FMGraphVertex<T>* From, FMGraphVertex<T>* To, bool Bidirectional = false)
	{
		if (Bidirectional)
		{
			return Connected(From, To, false) && Connected(To, From, false);
		}

		if (!From) return false;
		if (!To) return false;

		for (auto& C : From->Edges)
		{
			if (C.ToVertex == To)
			{
				return true;
			}
		}

		return false;
	}


	/** Compare nodes */
	bool AreEqual(const FMGraphVertex<T>* A, const FMGraphVertex<T>* B) const
	{
		if (!A) return false;
		if (!B) return false;
		return A->Id == B->Id;
	}

	/** Return the cost of an edge */
	float GetCost(FMGraphEdge<T>* Edge)
	{
		if (!Edge) return 0;

		if (Edge->Cost)
		{
			return Edge->Cost.GetValue();
		}

		Edge->Cost = Config.GetCost(&Edge->FromVertex->VertexData, &Edge->ToVertex->VertexData);
		return Edge->Cost.GetValue();
	}

	/** Clear any held data */
	void Reset()
	{
		VertexData.Reset();
		NextId = 0;
	}

	TArray<FMGraphVertex<T>*> GetVertexData()
	{
		TArray<FMGraphVertex<T>*> VertexPtr;
		for (int i = 0; i < VertexData.Num(); i++)
		{
			VertexPtr.Add(&*VertexData[i]);
		}
		return VertexPtr;
	}
};
