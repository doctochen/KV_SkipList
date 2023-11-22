#include <iostream>
#include <cstring>
#include <mutex>


#define STORE_FILE "dumpfile"

std::mutex mtx;
std::string delimiter = ":";

template <typename K, typename V>
class Node
{
public:
	Node(){}
	Node(K k, V v, int);
	~Node();

	K getKey()const;
	V getValue()const;
	void setValue(V);

	Node<K, V>** forward;
	int node_level;

private:
	K key;
	V value;
};

template<typename K, typename V>
Node<K, V>::Node(K k, V v, int level)
{
	this->key = k;
	this->value = v;
	node_level = level;

	this->forward = new Node<K, V>*[level + 1];
	memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
}

template<typename K, typename V>
Node<K, V>::~Node()
{
	for (int j = 0; j < node_level + 1; j++)
		delete[] forward[j];
	delete[] forward;
}

template<typename K, typename V>
inline K Node<K, V>::getKey() const
{
	return this->key;
}

template<typename K, typename V>
inline V Node<K, V>::getValue() const
{
	return value;
}

template<typename K, typename V>
inline void Node<K, V>::setValue(V value)
{
	this->value = value;
}



template <typename K, typename V>
class SkipList
{
public:
	SkipList(int);
	~SkipList();

	int getRandomLevel();

	Node<K£¬V>* create_node(K, V, int);

private:
	int _max_level;

	int _skip_list_level;

	Node<K, V>* header;

	std::ofstream _file_writer;
	std::ifstream _file_reader;

	int _element_count;
};