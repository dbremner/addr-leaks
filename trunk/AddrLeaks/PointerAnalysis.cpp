#include <iostream>

#include "PointerAnalysis.h"

const bool debug = false;
int teste = 1;

// ============================================= //

///  Default constructor
PointerAnalysis::PointerAnalysis()
{
    if (debug) std::cerr << "Initializing Pointer Analysis" << std::endl;
}

// ============================================= //

/// Destructor
PointerAnalysis::~PointerAnalysis()
{
    if (debug) std::cerr << "Terminating Pointer Analysis" << std::endl;
}

// ============================================= //

/**
 * Add a constraint of type: A = &B
 */
void PointerAnalysis::addAddr(int A, int B)
{
    if (debug) std::cerr << "Adding Addr Constraint: " <<  A << " = &" << B << std::endl;

	// Ensure nodes A and B exists.
	addNode(A);
	addNode(B);
	
	// Add B to pts(A)
	addToPts(B, A);
}

// ============================================= //

/**
 * Add a constraint of type: A = B
 */
void PointerAnalysis::addBase(int A, int B)
{
    if (debug) std::cerr << "Adding Base Constraint: " << A << " = " << B << std::endl;

	// Ensure nodes A and B exists.
	addNode(A);
	addNode(B);
	
	// Add edge from B to A
	addEdge(B, A);	
}

// ============================================= //

/**
 * Add a constraint of type: *A = B
 */
void PointerAnalysis::addStore(int A, int B)
{
    if (debug) std::cerr << "Adding Store Constraint: *" << A << " = " << B << std::endl;

	// Ensure nodes A and B exists.
	addNode(A);
	addNode(B);
	
	// Add the constraint
	stores[A].insert(B);
}

// ============================================= //

/**
 * Add a constraint of type: A = *B
 */
void PointerAnalysis::addLoad(int A, int B)
{
    if (debug) std::cerr << "Adding Load Constraint: " << A << " = *" << B << std::endl;

	// Ensure nodes A and B exists.
	addNode(A);
	addNode(B);
	
	// Add the constraint
	loads[B].insert(A);
}

// ============================================= //

/**
 * Return the set of positions pointed by A:
 *   pointsTo(A) = {B1, B2, ...}
 *  TODO: Check this
 */
std::set<int> PointerAnalysis::pointsTo(int A)
{
    if (debug) std::cerr << "Recovering Points-to-set of "<< A << std::endl;

	int repA = vertices[A];
	return pointsToSet[A];
}

// ============================================= //

/**
 * Add a new node to the graph if it doesn't already exist.
 */
void PointerAnalysis::addNode(int id)
{
    if (debug) std::cerr << "Adding Node " << id << std::endl;
    
	// Only add the node if it doesn't exist
	if (vertices.find(id) == vertices.end()) 
	{
		// Its current representative is itself and it is active
		vertices[id] = id;
		activeVertices.insert(id);
	}
}

// ============================================= //

/**
 * Add an edge in the graph.
 */
void PointerAnalysis::addEdge(int fromId, int toId)
{
    if (debug) std::cerr << "Adding Edge from "<< fromId << " to " << toId << std::endl;

	// We work with the representatives, so get them first.
	int repFrom = fromId;//vertices[fromId];
	int repTo = toId;//vertices[toId];
	
	// Add the edge (both directions)
	from[repFrom].insert(repTo);
    to[repTo].insert(repFrom);
}

// ============================================= //

/**
 * Add a node to the points-to set of another node
 */
void PointerAnalysis::addToPts(int pointed, int pointee)
{
    if (debug) std::cerr << "Adding " << pointed << " to pts(" << pointee << ")"  << std::endl;

	// Add the reference
	pointsToSet[pointee].insert(pointed);
}

// ============================================= //

void PointerAnalysis::removeCycles()
{
    if (debug) std::cerr << "Running cycle removal algorithm" << std::endl;

    // Some needed variables
    IntMap order;
    IntMap repr;
    int i = 0;
    IntSet current;
    IntDeque S;
    
    // At first, no node is visited and their representatives are themselves
    if (debug) std::cerr << ">> Initializing structures" << std::endl;
    IntSet::iterator V;
    for (V = activeVertices.begin(); V != activeVertices.end(); V++) 
	{
        order[*V] = 0;
        repr[*V] = *V;
    }

    // Visit all unvisited nodes
    if (debug) std::cerr << ">> Visiting vertices" << std::endl;
    for (V = activeVertices.begin(); V != activeVertices.end(); V++) 
	{
        if (order[*V] == 0) 
		{
            visit(*V, order, repr, i, current, S);
        }
    }
    
    // Merge those whose representatives are not themselves
    if (debug) std::cerr << ">> Merging vertices" << std::endl;
    for (V = activeVertices.begin(); V != activeVertices.end(); ) 
	{
        IntSet::iterator nextIt = V;
        nextIt++;
        if (debug) std::cerr << " * Current V (Before): " << *V << std::endl;
        if (debug) std::cerr << " * Current nextIt (Before): " << *nextIt << std::endl;
        if (repr[*V] != *V) 
		{
            merge(*V, repr[*V]);
        }
        V = nextIt;
        if (debug) std::cerr << " * Current V (After): " << *V << std::endl;
        if (debug) std::cerr << " * Current nextIt (After): " << *nextIt << std::endl;
    }

    if (debug) 
    {
        std::cerr << ">> Printing active vertices" << std::endl;
        for (V = activeVertices.begin(); V != activeVertices.end(); V++) 
        {
            std::cerr << *V << std::endl;
        }
    }
    
}

// ============================================= //

void PointerAnalysis::visit(int Node, IntMap& Order, IntMap& Repr, 
	int& idxOrder, IntSet& Curr, IntDeque& Stack)
{
    if (debug) std::cerr << "  - Visiting vertex " << Node << std::endl;

	idxOrder++;
    Order[Node] = idxOrder;

    IntSet::iterator w;
    for (w = from[Node].begin(); w != from[Node].end(); w++) 
	{
        if (Order[*w] == 0) visit(*w, Order, Repr, idxOrder, Curr, Stack);
        if (Curr.find(*w) == Curr.end()) 
		{
            Repr[Node] = (Order[Repr[Node]] < Order[Repr[*w]]) ? 
                Repr[Node] : 
                Repr[*w]
            ;
        }
    }

    if (Repr[Node] == Node) 
	{
        Curr.insert(Node);
        while (!Stack.empty()) 
		{
            int w = Stack.front();
            if (Order[w] <= Order[Node]) break;
            else 
			{
                Stack.pop_front();
                Curr.insert(w);
                Repr[w] = Node;
            }
        }
        // Push(TopologicalOrder, Node)
    }
    else 
	{
        Stack.push_front(Node);
    }
}
	
// ============================================= //

/**
 * Merge two nodes.
 * @param id the noded being merged
 * @param target the noded to merge into
 */
void PointerAnalysis::merge(int id, int target)
{
    if (debug) std::cerr << " - Merging " << id << " into " << target << std::endl;

	// Remove all edges id->target, target->id
    from[id].erase(target);
    to[target].erase(id);
    from[target].erase(id);
    to[id].erase(target);

    // Move all edges id->v to target->v
    if (debug) std::cerr << "Outgoing edges..." << std::endl;
    IntSet::iterator v;
    for (v = from[id].begin(); v != from[id].end(); v++) 
	{
        from[target].insert(*v);
        to[*v].erase(id);
        to[*v].insert(target);
    }
         
    // Move all edges v->id to v->target
    if (debug) std::cerr << "Incoming edges..." << std::endl;
    for (v = to[id].begin(); v != to[id].end(); v++) 
	{
        to[target].insert(*v);
        from[*v].erase(id);
        from[*v].insert(target);
    }

    // Mark the representative vertex
    vertices[id] = target;
    if (debug) std::cerr << "Removing vertice " << id << " from active." << std::endl;
	activeVertices.erase(id);

    // Merge Stores
    if (debug) std::cerr << "Stores..." << std::endl;
    for (v = stores[id].begin(); v != stores[id].end(); v++) 
	{
        stores[target].insert(*v);
    }
    stores[id].clear(); // Not really needed, I think

    // Merge Loads
    if (debug) std::cerr << "Loads..." << std::endl;
    for (v = loads[id].begin(); v != loads[id].end(); v++) 
	{
        loads[target].insert(*v);
    }
    loads[id].clear();

    // Join Points-To set
    if (debug) std::cerr << "Points-to-set..." << std::endl;
    for (v = pointsToSet[id].begin(); v != pointsToSet[id].end(); v++) 
	{
        pointsToSet[target].insert(*v);
    }
    if (debug) std::cerr << "End of merging..." << std::endl;
}

// ============================================= //

/**
 * Execute the pointer analysis
 * TODO: Add info about the analysis
 */
void PointerAnalysis::solve()
{
    std::set<std::string> R;
    IntSet WorkSet = activeVertices;
    IntSet NewWorkSet;
        
    if (debug) std::cerr << "Starting the analysis" << std::endl;

    while (!WorkSet.empty()) {
        int Node = *WorkSet.begin();
        WorkSet.erase(WorkSet.begin());
        
        if (debug) 
        {
            std::cerr << ">> New Step" << std::endl;
            std::cerr << " - Current Node: " << Node << std::endl;    
        }

        // For V in pts(Node)
        IntSet::iterator V;
        for (V = pointsToSet[Node].begin(); V != pointsToSet[Node].end(); V++ ) 
		{
            if (debug)
            {
                std::cerr << "   - Current V: " << *V << std::endl;
                std::cerr << "   - Load Constraints" << std::endl;
            }
            // For every constraint A = *Node
            IntSet::iterator A;
            for (A=loads[Node].begin(); A != loads[Node].end(); A++) 
            {
                // If V->A not in Graph
                if (from[*V].find(*A) == from[*V].end()) 
				{
                    addEdge(*V, *A);
                    NewWorkSet.insert(*V);
                }
            }

            if (debug) std::cerr << "   - Store Constraints" << std::endl;
            // For every constraint *Node = B
            IntSet::iterator B;
            for (B=stores[Node].begin(); B != stores[Node].end(); B++) 
            {
                // If B->V not in Graph
                if (from[*B].find(*V) == from[*B].end()) 
				{
                    addEdge(*B, *V);
                    NewWorkSet.insert(*B);
                }
            }
        }
		
        if (debug) std::cerr << " - End step" << std::endl;
        // For Node->Z in Graph
        IntSet::iterator Z = from[Node].begin();
        while (Z != from[Node].end() ) 
		{
            IntSet::iterator NextZ = Z;
            NextZ++;
            int ZVal = *Z;
            std::string edge = "" + Node;
            edge += "->";
            edge += ZVal;

            if (debug) std::cerr << " - Comparing pts" << std::endl;

            // Compare points-to sets
            if (pointsToSet[ZVal] == pointsToSet[Node] && R.find(edge) == R.end() ) 
			{
                if (debug) std::cerr << " - Removing cycles..." << std::endl;
                removeCycles();
                R.insert(edge);
                if (debug) std::cerr << " - Cycles removed" << std::endl;
            }

            // Merge the points-To Set
            if (debug) std::cerr << " - Merging pts" << std::endl;
            bool changed = false;
            for (V = pointsToSet[Node].begin(); V != pointsToSet[Node].end(); V++) 
			{
                changed |= pointsToSet[ZVal].insert(*V).second;
            }

            // Add Z to WorkSet if pointsToSet(Z) changed
            if (changed) 
			{
                NewWorkSet.insert(ZVal);
            }
    
            Z = NextZ;

            if (debug) std::cerr << " - End of step" << std::endl;
        }
        
        // Swap WorkSets if needed
        if (WorkSet.empty()) WorkSet.swap(NewWorkSet);
    }
}

// ============================================= //

/// Prints the graph to std output
void PointerAnalysis::print()
{
    std::cout << "# of Vertices: ";
    std::cout << vertices.size() << std::endl; 
    IntSet::iterator it;
    IntMap::iterator v;
	
    // Print Vertices Representatives
    std::cout << "Representatives: " << std::endl;
    for (v = vertices.begin(); v != vertices.end(); v++) 
	{
        std::cout << v->first << " -> ";
        std::cout << v->second << std::endl;
    }
    std::cout << std::endl;

    // Print Vertices Connections
    std::cout << "Connections (Graph): " << std::endl;
    for (it = activeVertices.begin(); it != activeVertices.end(); it++) 
	{
        std::cout << *it << " -> ";
        IntSet::iterator n;
        for (n = from[*it].begin(); n!= from[*it].end(); n++) 
        {
            std::cout << *n << " ";
        }
        //std::cout << "0";
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // Prints PointsTo sets
    std::cout << "Points-to-set: " << std::endl;
    for (v = vertices.begin(); v != vertices.end(); v++) 
	{
        std::cout << v->first << " -> {";
        IntSet::iterator n;
        for (n = pointsToSet[v->first].begin(); n != pointsToSet[v->first].end();  n++) 
        {
            std::cout << *n << ", ";
        }

        std::cout << "}";
        std::cout << std::endl;
    }
    std::cout << std::endl;

}

// ============================================= //

/// Returns the points-to map
std::map<int, std::set<int> > PointerAnalysis::allPointsTo() {
    return pointsToSet;
}

