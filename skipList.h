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
Node<K, V>::Node(K k, V v, const int level)
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
	bool searchElement(K);
	void deleteElement(K);
	void dumpFile();
	void loadFile();
	void clear(Node<K, V>*);
	int size();

private:
	void get_key_value_form_string(const std::string& str, std::string* key, std::string* value);
	bool is_valid_string(const std::string& str);

private:
	const int _max_level;

	int _skip_list_level;

	Node<K, V>* header;

	std::ofstream _file_writer;
	std::ifstream _file_reader;

	int _element_count;
};

template<typename K, typename V>
SkipList<K, V>::SkipList(int maxLevel): _max_level(maxLevel)
{
	this->_skip_list_level = 0;
	this->_element_count = 0;

	K k;
	V v;
	this->header = new Node<K, V>(k, v, _max_level);
}

template<typename K, typename V>
SkipList<K, V>::~SkipList()
{
	if (_file_writer.is_open())
		_file_writer.close();
	if (_file_reader.is_open())
		_file_reader.close();

	if (header->forward[0] != nullptr)
	{
		clear(header->forward[0]);
	}
	delete header;
}

template<typename K, typename V>
int SkipList<K, V>::getRandomLevel()
{
	int k = 0;
	for (;;)
	{
		if (rand() % 2 == 0)
		{
			return k;
		}
		k++;
		if (k == _max_level) return k;
	}
}

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
bool SkipList<K, V>::searchElement(K key)
{
	std::cout << "searchElement------------------\n";
	Node<K, V>* current = header;

	for (int j = _skip_list_level; j >= 0; j--)
	{
		while (current != nullptr && current->forward[j]->getkey() < key)
			current = current->forward[j];
	}
	current = current->forward[0];
	if (current != nullptr && current->getKey() == Key)
	{
		std::cout << "found key: " << current->getKey() << " value: " << current->getValue() << std::endl;
		return true;
	}
	std::cout << "Not Found Key" << std::endl;
	return false;
}

template<typename K, typename V>
void SkipList<K, V>::deleteElement(K key)
{
	std::unique_lock<std::mutex> lock(mtx);
	Node<K, V>* current = header;
	Node<K, V>* updateNode[_max_level + 1];
	memset(updateNode, 0, sizeof(Node<K, V>*) * (_max_level + 1));

	for (int j = _skip_list_level; j >= 0; j--)
	{
		while (current->forward[j] != nullptr && current->forward[j]->getkey() < key)
			current = current->forward[j];
		updateNode[j] = current;
	}

	current = current->forward[0];

	if (curren != nullptr && current->getKey() == key)
	{
		for (int j = 0; j <= _skip_list_level; j++)
		{
			if (updateNode[j]->forward[j] != current)
				break;
			updateNode[j]->forward[j] = current->forward[j];
		}

		while (_skip_list_level > 0 && header->forward[_skip_list_level] == 0)
			_skip_list_level--;

		std::cout << "Successfully deleted key " << key << std::endl;
		delete current;
		_element_count--;
	}
	return;
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

template<typename K, typename V>
void SkipList<K, V>::clear(Node<K, V>* cur)
{
	if (cur->forward[0] != nullptr)
		clear(cur->forward[0]);
	delete cur;
}

template<typename K, typename V>
int SkipList<K, V>::size()
{
	return _element_count;
}

template<typename K, typename V>
void SkipList<K, V>::get_key_value_form_string(const std::string& str, std::string* key, std::string* value)
{
	if (!is_valid_string(str)) return;
	*key = str.substr(0, str.find(delimiter));
	*value = str.substr(str.find(delimiter) + 1, str.length());
}

template<typename K, typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str)
{
	if (str.empty()) return false;
	if (str.find(delimiter) == std::string::npos) return false;
	return true;
}


