// BoostWrapper.cpp : Defines the exported functions for the DLL application.

#include "cstdint"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/boyer_myrvold_planar_test.hpp"
#include "boost/graph/planar_face_traversal.hpp"
#include <vector>

#define TRADITIONALDLL_EXPORTS  
#ifdef TRADITIONALDLL_EXPORTS  
#define TRADITIONALDLL_API __declspec(dllexport)  
#else  
#define TRADITIONALDLL_API __declspec(dllimport)  
#endif  

extern "C" {
	TRADITIONALDLL_API bool GetFaces(std::int32_t * edges, std::int32_t edgesCount, std::int32_t verticesCount, std::int32_t * facesPtr, std::int32_t * facesBordersPtr, std::int32_t & facesCount);

	TRADITIONALDLL_API bool IsPlanar(std::int32_t * edges, std::int32_t edgesCount, std::int32_t verticesCount);
}

struct output_visitor : public boost::planar_face_traversal_visitor
{
	std::vector<std::vector<int>> faces;
	std::vector<int> last_face;

	void begin_face()
	{
		/* empty */
	}

	void next_vertex(int v)
	{
		last_face.push_back(v);
	}

	void end_face()
	{
		faces.push_back(last_face);
		last_face.clear();
	}
};


// Gets faces of a given planar graph
// Returns whether the computation was successful
bool GetFaces(std::int32_t * edges, std::int32_t edgesCount, std::int32_t verticesCount, std::int32_t * facesPtr, std::int32_t * facesBordersPtr, std::int32_t & facesCount)
{
	using namespace boost;

	typedef adjacency_list
		< vecS,
		vecS,
		undirectedS,
		property<vertex_index_t, int>,
		property<edge_index_t, int>
		>
		graph;

	graph g(verticesCount);

	// The number of edges must be divisible by 2 because edges are given as pair of integers
	if (edgesCount % 2 != 0)
	{
		return false;
	}

	for (auto i = 0; i < edgesCount / 2; ++i)
	{
		boost::add_edge(edges[2 * i], edges[2 * i + 1], g);
	}

	// Initialize the interior edge index
	property_map<graph, edge_index_t>::type e_index = get(edge_index, g);
	graph_traits<graph>::edges_size_type edge_count = 0;
	graph_traits<graph>::edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei)
	{
		put(e_index, *ei, edge_count++);
	}
		
	// Test for planarity - we know it is planar, we just want to 
	// compute the planar embedding as a side-effect
	typedef std::vector< graph_traits<graph>::edge_descriptor > vec_t;
	std::vector<vec_t> embedding(num_vertices(g));
	if (!boyer_myrvold_planarity_test(boyer_myrvold_params::graph = g,
		boyer_myrvold_params::embedding =
		&embedding[0]
	))
	{
		return false;
	}

	// Travers the graph
	output_visitor my_visitor{};
	boost::planar_face_traversal(g, &embedding[0], my_visitor);

	// Prepare faces for C# code
	std::vector<int32_t> faces{};
	std::vector<int32_t> facesBorders{};

	auto counter = 0;
	auto counterFace = 0;

	for (size_t i = 0; i < my_visitor.faces.size(); i++)
	{
		auto face_raw = my_visitor.faces[i];

		for (size_t j = 0; j < face_raw.size(); j++)
		{
			facesPtr[counter++] = face_raw[j];
		}

		facesBordersPtr[counterFace++] = face_raw.size();
	}

	facesCount = my_visitor.faces.size();

	return true;
}

// Check whether a given graph is planar
bool IsPlanar(std::int32_t * edges, std::int32_t edgesCount, std::int32_t verticesCount)
{
	using namespace boost;

	typedef adjacency_list<vecS,
		vecS,
		undirectedS,
		property<vertex_index_t, int>
	> graph;

	graph g(verticesCount);

	// The number of edges must be divisible by 2 because edges are given as pair of integers
	if (edgesCount % 2 != 0)
	{
		return false;
	}

	for (auto i = 0; i < edgesCount / 2; ++i)
	{
		boost::add_edge(edges[2 * i], edges[2 * i + 1], g);
	}

	// Check planarity
	if (!boyer_myrvold_planarity_test(g))
	{
		return false;
	}

	return true;
}