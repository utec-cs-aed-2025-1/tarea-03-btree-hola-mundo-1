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

  int height(){ //altura del arbol. Considerar altura 0 para arbol vacio
    if(root == nullptr)
      return 0;

    int height = 0;
    Node<TK>* temp = root;

    while(!temp->leaf){
      temp = temp->children[0];
      height += 1;
    }

    return height;
  };
  
  // recorrido inorder
  string toString(const string& sep){
    return toString(root, sep);
  };
  
  vector<TK> rangeSearch(TK begin, TK end){
    return range_search(root, begin, end);
  };

  TK minKey(){ // minimo valor de la llave en el arbol
    if (root == nullptr) {
      throw "error, arbol nulo";
    }
    Node<TK>* temp = root;
    while(temp->leaf == false){
        temp = temp->children[0]
      }
      return temp->keys[0];
    };
     
  TK maxKey(){ // maximo valor de la llave en el arbol
    if (root == nullptr) {
      throw "error, arbol nulo";
    }
    Node<TK>* temp = root;
    while(temp->leaf == false){
      temp = temp->children[temp->count];
    }
    return temp->keys[temp->count-1];
  };

  void clear_node(Node<TK>* nodo){
    //caso base
    if(nodo == nullptr) 
      return;
    
    //recursividad
    if(!nodo->leaf){
        for(int i = 0; i <= nodo->count; i++){
            clear_node(nodo->children[i]);
        }
    }

    delete nodo;
  }
  void clear(){ // eliminar todos lo elementos del arbol
    clear_node(root);
    root = nullptr;
    n = 0;
    return;
  }; 
  
  int size(){ // retorna el total de elementos insertados
    return n;
  }   
  

  // Construya un árbol B a partir de un vector de elementos ordenados
  static BTree* build_from_ordered_vector(vector<TK> elements, int M){
    BTree* resultado = new BTree(M);
    for(auto element:elements){
      resultado->insert(element);
    }
    return resultado;
  }


  // Verifique las propiedades de un árbol B
  //Propiedades: 
  //➢ La raíz tiene al menos 2 hijos y contiene al menos un key
  //➢ Todas las hojas están al mismo nivel
  //➢ En cada nodo, todas las keys están ordenadas
  //➢ Si M es el orden del árbol, entonces:
  //➢ Cada nodo excepto la raíz, ⌈M/2⌉ - 1<= numero de keys <= M - 1
  //➢ Cada nodo interno excepto la raiz, ⌈M/2⌉ <= número de hijos <= M
  bool check_properties();
  

  ~BTree(){
    if (this->root != nullptr){
      clear();
    }
  };     // liberar memoria
};

#endif
