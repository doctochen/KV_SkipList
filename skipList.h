#include <iostream>
#include <cstring>
#include <mutex>
#include <fstream>
#include <string>


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

	Node<K,V>* create_node(K, V, int);
	int insertElement(K, V);
	void displayList();
	bool searchElement();
	void deleteElement();
	void dumpFile();
	void loadFile();
	void clear(Node<K, V>*);
	int size();

private:
	void get_key_value_form_string(const std::string& str, std::string* key, std::string* value);


private:
	int _max_level;

	int _skip_list_level;

	Node<K, V>* header;

	std::ofstream _file_writer;
	std::ifstream _file_reader;

	int _element_count;
};

template<typename K, typename V>
Node<K,V>* SkipList<K, V>::create_node(const K k,const V v, int level)
{
	Node<K, V> n = new Node<K, V>(k, v, level);
	return n;
}

template<typename K, typename V>
int SkipList<K, V>::insertElement(const K key, const V value)
{
	std::unique_lock<std::mutex> lock(mtx);
	Node<K, V>* current = header;

	Node<K, V>* updateNode[_max_level + 1];
	memset(updateNode, 0, sizeof(Node<K, V>*) * (_max_level + 1));

	for (int j = _skip_list_level; j >= 0; j--)
	{
		if (nullptr != current && current->getKey() < key)
			current = current->forward[j];
		updateNode[j] = current;
	}

	current = current->forward[0];

	if (current != nullptr && current->getKey() == key)
	{
		std::cout << "key has exist" << std::endl;
		return 1;
	}

	if (current != nullptr || current->getKey != key)
	{
		int randomLevel = getRandomLevel();

		if (randomLevel > _skip_list_level)
		{
			for (int j = _skip_list_level + 1; j < randomLevel + 1; j++)
				updateNode[j] = header;
			_skip_list_level = randomLevel;
		}

		Node<K, V>* inserted_node = create_node(key, value, randomLevel);

		for (int j = 0; j < randomLevel; j++)
		{
			inserted_node->forward[j] = updateNode[j]->forward[j];
			updateNode[j]->forward[j] = inserted_node;
		}

		std::cout << "insert successfully key" << std::endl;
		_element_count++;
	}
	return 0;
}

template<typename K, typename V>
void SkipList<K, V>::displayList()
{
	std::cout << "\n***********SkipList***********\n";
	for (int j = 0; j <= _skip_list_level; j++)
	{
		Node<K, V> node = header->forward[j];
		std::cout << "level " << j << endl;
		while (node != nullptr)
		{
			std::cout << node->getKey() << ":" << node->getValue() << ";";
			node = node->forward[j];
		}
		std::cout << std::endl;
	}
}

template<typename K, typename V>
void SkipList<K, V>::dumpFile()
{
	std::cout << "dump file------------------" << std::endl;
	_file_writer.open(STORE_FILE);
	Node<K, V>* node = this->header->forward[0];

	while (node != nullptr)
	{
		_file_writer << node->getKey() << ":" << node->getValue() << "\n";
		std::cout << node->getKey() << ":" << node->getValue() << ";\n";
		node = node->forward[0];
	}

	_file_writer.flush();
	_file_writer.close();
	return;
}

template<typename K, typename V>
void SkipList<K, V>::loadFile()
{
	_file_reader.open(STORE_FILE);
	std::cout << "load_file------------" << std::endl;
	std::string line;
	std::string* key = new std::string();
	std::string* value = new std::string();

	while (getline(_file_reader, line))
	{
		get_key_value_form_string(line, key, value);
		if (key->empty() || value->empty()) continue;

		insertElement(stoi(*key), *value);
		std::cout << "key:" << *key << "value:" << *value << std::endl;
	}
	delete key;
	delete value;
	_file_reader.close();
}
