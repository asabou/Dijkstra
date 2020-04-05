#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <chrono>

#define MAX 1000

using namespace std;
using namespace std::chrono;

int n, m; //no of nodes and edges
int source, destination; 
int p[MAX]; //the vector of "fathers"
int dist[MAX]; //vector of distances

/*the struct of the node*/
struct Node {
	int y;
	int cost;
	Node(const int& y, const int& cost) {
		this->y = y;
		this->cost = cost;
	}
	/*overriding the < operator*/
	bool operator < (const Node& other) const {
		return this->cost > other.cost;
	}
};

vector<Node>G[MAX]; //adjacency

/*read data from the given file as parameter and populate de adjacency*/
void readData(const string& filename) {
	ifstream f(filename);
	f >> n >> m;
	f >> source >> destination;
	int x, y, cost;
	for (int i = 1; i <= m; i++) {
		f >> x >> y >> cost;
		G[x].push_back(Node(y, cost));
	}
	f.close();
}

/*print data from the file
* just to check if the read is well done
*/
void printData() {
	cout << "n : " << n << " m: " << m << endl;
	cout << "source : " << source << " destination: " << destination << endl;
	cout << "Adjacency: \n";
	for (int i = 1; i <= n; i++) {
		cout << "The neighbours of node  " << i << "  are: ";
		for (const auto& node : G[i]) {
			cout << node.y << " ";
		}
		cout << endl;
	}
}

/*the algorithm*/
void dijkstra() {
	//init the vector of "fathers" and the vector of distances
	for (int i = 1; i <= n; i++) {
		dist[i] = MAX;
		p[i] = -1;
	}
	dist[source] = 0;
	priority_queue<Node>Q;
	Q.push(Node(source, dist[source]));
	while (!Q.empty()) {
		int node = Q.top().y;
		Q.pop();
		vector<Node>::iterator it;
		//foreach neighbour of the node from the top of priority_queue
		for (it = G[node].begin(); it != G[node].end(); it++) {
			if (dist[it->y] > dist[node] + it->cost) { //if there is a shorter path, update it and push that node to the priority queue
				dist[it->y] = dist[node] + it->cost;
				p[it->y] = node;
				Q.push(Node(it->y, dist[it->y]));
			}
		}
	}
	G->clear();
}

/*print the array of "fathers" just to check if the algorithm works*/
void printFathers() {
	for (int i = 1; i <= n; i++) {
		cout << p[i] << " ";
	}
	cout << endl;
}

/*print the array of the distances just to check if the algorithm works*/
void printDist() {
	for (int i = 1; i <= n; i++) {
		cout << source << " --> " <<i<<" : "<< dist[i] << "\n";
	}
}

/*print the desired otput
the cost from source and destination
the path from source to destination
*/
void printToFile(const string& filename) {
	ofstream g(filename);
	if (dist[destination] == MAX) {
		g << "There is no path between source and destination";
	}
	else {
		g << dist[destination] << endl;
		stack<int>s;
		s.push(destination);
		int father = p[destination];
		while (father != -1) {
			s.push(father);
			father = p[father];
		}
		while (!s.empty()) {
			g << s.top() << " ";
			s.pop();
		}
	}
	g.close();
}

void printToTimeFile(long microseconds) {
	ofstream g("time.csv", std::ios_base::app);
	g << n << "," << m << "," <<source<<","<<destination<<","<< microseconds << endl;
	g.close();
}

void runDijkstraEasy() {
	readData("easy_input.txt");
	printData();
	auto start = high_resolution_clock::now();
	dijkstra();
	auto end = high_resolution_clock::now();
	auto time = duration_cast<microseconds>(end - start);
	printDist();
	printFathers();
	printToTimeFile(time.count());
	printToFile("easy_output.txt");
}

void runDijkstraHard() {
	readData("hard_input.txt");
	printData();
	auto start = high_resolution_clock::now();
	dijkstra();
	auto end = high_resolution_clock::now();
	auto time = duration_cast<microseconds>(end - start);
	printDist();
	printFathers();
	printToTimeFile(time.count());
	printToFile("hard_output.txt");
}

void startDijkstra() {
	runDijkstraEasy();
	runDijkstraHard();
}

int main() {
	startDijkstra();
	return 0;
}