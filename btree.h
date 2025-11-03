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
  }

  vector<TK> range_search(Node<TK>* node, TK begin, TK end){
    vector<TK> output;
    if (node == nullptr) return output;

    for (int i = 0; i < node->count; i++) {
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
  }

  string toString(Node<TK>* node, const string& sep){
    if (node == nullptr) return "";

    string output = "";

    for (int i = 0; i < node->count; i++) {
      if (!node->leaf){
        output += toString(node->children[i], sep);
      }
      output += to_string(node->keys[i]) + sep;
    }

    if (!node->leaf){
      output += toString(node->children[node->count], sep);
    }
    
    return output;
  }
  //funciones auxiliares para verificar las propiedades del arbol
  bool verificar_keys_ordenadas(Node<TK>* nodo) {
    if (nodo == nullptr) return true;
    
    for (int i = 0; i < nodo->count - 1; i++) {
      if (nodo->keys[i] >= nodo->keys[i + 1]) {
        return false;
      }
    }
    
    if (!nodo->leaf) {
      for (int i = 0; i <= nodo->count; i++) {
        if (!verificar_keys_ordenadas(nodo->children[i])) {
          return false;
        }
      }
    }
    
    return true;
  }

  bool verificar_altura_hojas(Node<TK>* nodo) {
    if (nodo == nullptr) return true;
        int altura_esperada = 0;
    Node<TK>* temp = nodo;
    while (!temp->leaf) {
      temp = temp->children[0];
      altura_esperada++;
    }
    
    return verificar_todas_hojas_mismo_nivel(nodo, 0, altura_esperada);
  }
  
  bool verificar_todas_hojas_mismo_nivel(Node<TK>* nodo, int nivel_actual, int altura_esperada) {
    if (nodo == nullptr) return true;
    
    if (nodo->leaf) {
      return nivel_actual == altura_esperada;
    }
    
    for (int i = 0; i <= nodo->count; i++) {
      if (!verificar_todas_hojas_mismo_nivel(nodo->children[i], nivel_actual + 1, altura_esperada)) {
        return false;
      }
    }
    
    return true;
  }
  
  bool verificar_limites_nodos(Node<TK>* nodo, bool es_raiz) {
    if (nodo == nullptr) return true;

    //Calculo de los limites en base a M
    int min_keys = (M + 1) / 2 - 1;
    int max_keys = M - 1;
    int min_children = (M + 1) / 2;
    int max_children = M;
    
    if (!es_raiz) {
      // verificar límites de keys en nodos internos que no son la raiz
      if (nodo->count < min_keys || nodo->count > max_keys) {
        return false;
      }
      
      // verificar límites de children en nodos internos que no son la raiz
      if (!nodo->leaf) {
        int num_children = nodo->count + 1;
        if (num_children < min_children || num_children > max_children) {
          return false;
        }
      }
    } else {
      // verificar que la raiz tiene al menos 1 key y máximo M-1 keys
      if (nodo->count < 1 || nodo->count > max_keys) {
        return false;
      }
      
      // verificar que la raiz interna tenga al menos 2 hijos y máximo M hijos
      if (!nodo->leaf) {
        int num_children = nodo->count + 1;
        if (num_children < 2 || num_children > max_children) {
          return false;
        }
      }
    }
    
    // Recursivamente verificar todos los hijos
    if (!nodo->leaf) {
      for (int i = 0; i <= nodo->count; i++) {
        if (!verificar_limites_nodos(nodo->children[i], false)) {
          return false;
        }
      }
    }
    
    return true;
  }

 public:
  BTree(int _M) : root(nullptr), M(_M), n(0) {}

  //indica si se encuentra o no un elemento
  bool search(TK key){
    return this->search(this->root, key);
  }

  //Funcion auxiliar para hacer divisiones en el insert
  void splitChild(Node<TK>* parent, int childIndex) {

    Node<TK>* fullChild = parent->children[childIndex];
    
    Node<TK>* newChild = new Node<TK>(M);
    newChild->leaf = fullChild->leaf;
    
    int mid = (M - 1) / 2;
    TK midKey = fullChild->keys[mid];
    
    newChild->count = M - 1 - mid - 1;
    for (int i = 0; i < newChild->count; i++) {
        newChild->keys[i] = fullChild->keys[mid + 1 + i];
    }
    
    if (!fullChild->leaf) {
        for (int i = 0; i <= newChild->count; i++) {
            newChild->children[i] = fullChild->children[mid + 1 + i];
        }
    }
    
    fullChild->count = mid;
    
    for (int i = parent->count; i > childIndex; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[childIndex + 1] = newChild;
    
    for (int i = parent->count - 1; i >= childIndex; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }
    parent->keys[childIndex] = midKey;
    parent->count++;
  }

  void insert(TK key){ //inserta un elemento

    // si el árbol está vacío, crear raíz
    if (root == nullptr) {
        root = new Node<TK>(this->M);
        root->keys[0] = key;
        root->count = 1;
        root->leaf = true;
        n++;
        return;
    }

    // si la raíz está llena, crear nueva raíz y dividir
    if (root->count == M - 1) {
        Node<TK>* newRoot = new Node<TK>(M);
        newRoot->leaf = false;
        newRoot->children[0] = root;
        root = newRoot;
        
        // dividir la antigua raíz
        splitChild(newRoot, 0);
    }

    // encontramos la hoja donde insertar
    Node<TK>* current = root;
    while (!current->leaf) {
        int i = 0;
        while (i < current->count && key > current->keys[i]) {
            i++;
        }
        
        // si el hijo está lleno, dividirlo primero
        if (current->children[i]->count == M - 1) {
            splitChild(current, i);
          
            if (key > current->keys[i]) {
                i++;
            }
        }
        
        current = current->children[i];
    }
    
    // insertar la clave
    int pos = current->count;
    while (pos > 0 && key < current->keys[pos-1]) {
        current->keys[pos] = current->keys[pos-1];
        pos--;
    }
    current->keys[pos] = key;
    current->count++;
    n++;
  }


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
  }
  
  // recorrido inorder
  string toString(const string& sep){
    return toString(root, sep);
  }
  
  vector<TK> rangeSearch(TK begin, TK end){
    return range_search(root, begin, end);
  }

  TK minKey(){ // minimo valor de la llave en el arbol
    if (root == nullptr) {
      throw "error, arbol nulo";
    }
    Node<TK>* temp = root;
    while(temp->leaf == false){
        temp = temp->children[0];
      }
      return temp->keys[0];
    }
     
  TK maxKey(){ // maximo valor de la llave en el arbol
    if (root == nullptr) {
      throw "error, arbol nulo";
    }
    Node<TK>* temp = root;
    while(temp->leaf == false){
      temp = temp->children[temp->count];
    }
    return temp->keys[temp->count-1];
  }

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
  } 
  
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
  bool check_properties(){
    
    if (root == nullptr){
      return true;
    }
    
    //verificar keys ordenadas
    if (!verificar_keys_ordenadas(root)){
      return false;
    }
    
    // verificar que todas las hojas estén al mismo nivel
    if (!verificar_altura_hojas(root)) {
      return false;
    }
    
    // verificar límites de keys y children para los nodos
    if (!verificar_limites_nodos(root, true)) {
      return false;
    }
    
    return true;
  }



public:
  ~BTree(){
    if (this->root != nullptr){
      clear();
    }
  };     // liberar memoria
};

#endif
