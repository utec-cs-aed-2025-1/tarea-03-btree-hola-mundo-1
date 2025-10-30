#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include <string>
#include "node.h"

using namespace std;

template <typename TK>
class BTree {
 private:
  Node<TK>* root;
  int M;  // grado u orden del arbol
  int n; // total de elementos en el arbol 

  //helper functions
  private:
  bool search(Node<TK>* node, TK key){
    int i = 0;
    while (i < node->count && key > node->keys[i]) i++;
    if (key == node->keys[i]) {
      return true;
    }else if (!node -> leaf) {
      return search(node->children[i], key);
    } else {
      return false;
    }
  };

  vector<TK> range_search(Node<TK>* node, TK begin, TK end){
    vector<TK> output;
    if (node == nullptr) return output;

    for (int i = 0; i < node->count, i++) {
      TK k = node->keys[i];

      //verificamos otros hijos
      if (node->children[i] != nullptr && k > begin) {
        vector<TK> other = range_search(node->children[i], begin, end);
        output.insert(output.end(), other.begin(), other.end());
      }

      //insertamos el valor
      if (k >= begin && k <= end) {
        output.push_back(node->keys[i]);
      }
    }

    //verificamos el de la derecha
    if (node->children[node->count] != nullptr && node->keys[node->count - 1] < end) {
      vector<TK> right = range_search(node->children[node->count], begin, end);
      output.insert(output.end(), right.begin(), right.end());
    }

    return output;
  };

  string toString(Node<TK>* node, const string& sep){
    if (node == nullptr) return "";

    string output = "";

    for (int i = 0; i < node->count; i++) {
      if (!node->leaf){
        output += toString(node->children[i], sep);
        output += to_string(node->keys[i]) + sep;
      }
    }

    if (!node->leaf){
      output += toString(node->children[node->count], sep)
    }
    
    return output;
  }

 public:
  BTree(int _M) : root(nullptr), M(_M) {}

  //indica si se encuentra o no un elemento
  bool search(TK key){
    return this->search(this->root, key);
  };

  void insert(TK key);//inserta un elemento
  void remove(TK key);//elimina un elemento
  int height();//altura del arbol. Considerar altura 0 para arbol vacio
  
  // recorrido inorder
  string toString(const string& sep){
    return toString(root, sep);
  };
  
  vector<TK> rangeSearch(TK begin, TK end){
    return range_search(root, begin, end);
  };

  TK minKey();  // minimo valor de la llave en el arbol
  TK maxKey();  // maximo valor de la llave en el arbol
  void clear(); // eliminar todos lo elementos del arbol
  int size(); // retorna el total de elementos insertados  
  
  // Construya un árbol B a partir de un vector de elementos ordenados
  static BTree* build_from_ordered_vector(vector<T> elements);
  // Verifique las propiedades de un árbol B
  bool check_properties();

  ~BTree(){
    if (this->root != nullptr){
      delete this->root;
    }
  };     // liberar memoria
};

#endif
