#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include <string>
#include <queue>
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

  string toString(Node<TK>* node, const string& sep, int depth = 0){
    if (node == nullptr) return "";
    if (node->count == 0) return ""; // Ignorar nodos vacíos
    if (depth > 100) return ""; // Protección contra loops

    string output = "";

    for (int i = 0; i < node->count; i++) {
      if (!node->leaf && node->children[i] != nullptr){
        output += toString(node->children[i], sep, depth + 1);
      }
      output += to_string(node->keys[i]) + sep;
    }

    if (!node->leaf && node->children[node->count] != nullptr){
      output += toString(node->children[node->count], sep, depth + 1);
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

  TK get_successor(Node<TK>* node) {
    while (!node->leaf) {
      node = node->children[0];
    }
    return node->keys[0];
  }
  
  // tomar una key del hermano izquierdo
  void pedir_prestado_izquierda(Node<TK>* padre, int idx_hijo) {
    Node<TK>* hijo = padre->children[idx_hijo];
    Node<TK>* hermano_izquierdo = padre->children[idx_hijo - 1];
    
    // Desplazar todas las keys del hijo una pos a la der
    for (int i = hijo->count; i > 0; i--) {
      hijo->keys[i] = hijo->keys[i - 1];
    }
    
    // desplazar hijos si no es hoja
    if (!hijo->leaf) {
      for (int i = hijo->count + 1; i > 0; i--) {
        hijo->children[i] = hijo->children[i - 1];
      }
    }

    // La key del padre baja al hijo
    hijo->keys[0] = padre->keys[idx_hijo - 1];
    hijo->count++;
    
    // La ultima key del hermano sube al padre
    padre->keys[idx_hijo - 1] = hermano_izquierdo->keys[hermano_izquierdo->count - 1];
    
    if (!hijo->leaf) {
      hijo->children[0] = hermano_izquierdo->children[hermano_izquierdo->count];
      hermano_izquierdo->children[hermano_izquierdo->count] = nullptr;
    }
    hermano_izquierdo->count--;
  }

  // tomar una key del hermano derecho
  void pedir_prestado_derecha(Node<TK>* padre, int idx_hijo) {
    Node<TK>* hijo = padre->children[idx_hijo];
    Node<TK>* hermano_derecho = padre->children[idx_hijo + 1];
    
    hijo->keys[hijo->count] = padre->keys[idx_hijo];
    hijo->count++;
    
    padre->keys[idx_hijo] = hermano_derecho->keys[0];
    
    if (!hijo->leaf) {
      hijo->children[hijo->count] = hermano_derecho->children[0];
    }
    
    for (int i = 0; i < hermano_derecho->count - 1; i++) {
      hermano_derecho->keys[i] = hermano_derecho->keys[i + 1];
    }
    
    if (!hermano_derecho->leaf) {
      for (int i = 0; i < hermano_derecho->count; i++) {
        hermano_derecho->children[i] = hermano_derecho->children[i + 1];
      }
      hermano_derecho->children[hermano_derecho->count] = nullptr;
    }
    hermano_derecho->count--;
  }
  
  // Fusionar hijo con su hermano izquierdo
  void fusionar_con_izquierda(Node<TK>* padre, int idx_hijo) {
    Node<TK>* nodo_actual = padre->children[idx_hijo];
    Node<TK>* nodo_izq = padre->children[idx_hijo - 1];
    
    int pos_inicial = nodo_izq->count;

    nodo_izq->keys[pos_inicial++] = padre->keys[idx_hijo - 1];
    
    int j = 0;
    while (j < nodo_actual->count) {
      nodo_izq->keys[pos_inicial + j] = nodo_actual->keys[j];
      j++;
    }
    
    nodo_izq->count = pos_inicial + nodo_actual->count;
    
    if (!nodo_actual->leaf) {
      int base_idx = pos_inicial;
      for (int k = 0; k <= nodo_actual->count; k++) {
        nodo_izq->children[base_idx + k] = nodo_actual->children[k];
        nodo_actual->children[k] = nullptr;
      }
    }
    
    int pos = idx_hijo - 1;
    while (pos < padre->count - 1) {
      padre->keys[pos] = padre->keys[pos + 1];
      pos++;
    }
    
    int p = idx_hijo;
    while (p < padre->count) {
      padre->children[p] = padre->children[p + 1];
      p++;
    }
    
    padre->children[padre->count] = nullptr;
    padre->count--;
    delete nodo_actual;
  }

  // Fusionar hijo con su hermano derecho
  void fusionar_con_derecha(Node<TK>* padre, int idx_hijo) {
    Node<TK>* nodo_izq = padre->children[idx_hijo];
    Node<TK>* nodo_der = padre->children[idx_hijo + 1];

    // nueva pos inicial
    int pos_base = nodo_izq->count;
    
    // insertamos la key del padre
    nodo_izq->keys[pos_base] = padre->keys[idx_hijo];
    
    // copiamos las keys del hermano derecho
    int offset = pos_base + 1;
    for (int k = 0; k < nodo_der->count; k++) {
      nodo_izq->keys[offset + k] = nodo_der->keys[k];
    }
    
    nodo_izq->count = offset + nodo_der->count;

    // transferimos punteros a hijos si es nodo interno
    if (!nodo_izq->leaf) {
      int idx_child = pos_base + 1;
      int total_hijos = nodo_der->count + 1;
      for (int m = 0; m < total_hijos; m++) {
        nodo_izq->children[idx_child + m] = nodo_der->children[m];
        nodo_der->children[m] = nullptr;
      }
    }

    // eliminamos la key del padre desplazandonos hacia la izquierda
    int idx_key = idx_hijo;
    while (idx_key + 1 < padre->count) {
      padre->keys[idx_key] = padre->keys[idx_key + 1];
      idx_key++;
    }
    
    int idx_ptr = idx_hijo + 1;
    while (idx_ptr < padre->count) {
      padre->children[idx_ptr] = padre->children[idx_ptr + 1];
      idx_ptr++;
    }
    
    padre->children[padre->count] = nullptr;
    padre->count--;
    delete nodo_der;
  }
  
  void fix_children_remove(Node<TK>* padre, int idx_hijo) {
    int min_claves = (M + 1) / 2 - 1;
    
    // Intentar pedir prestado del hermano izquierdo
    if (idx_hijo > 0 && padre->children[idx_hijo - 1]->count > min_claves) {
      pedir_prestado_izquierda(padre, idx_hijo);
      return;
    }
    
    // Intentar pedir prestado del hermano derecho
    if (idx_hijo < padre->count && padre->children[idx_hijo + 1]->count > min_claves) {
      pedir_prestado_derecha(padre, idx_hijo);
      return;
    }
    
    // No se puede pedir prestado, hacer fusión
    if (idx_hijo > 0) {
      fusionar_con_izquierda(padre, idx_hijo);
    } else {
      fusionar_con_derecha(padre, idx_hijo);
    }
  }

  bool remove_recursion(Node<TK>* node, TK key) {
    // Caso base: nodo nulo
    if (!node) return false;
    
    // calcula el minimo de keys permitido para un nodo q no sea raiz
    int min_keys = (M + 1) / 2 - 1;


    // 0. Buscar la posicion de la key en el nodo actual
    int idx = 0;
    while (idx < node->count && node->keys[idx] < key) {
      idx++;
    }
    // ver si la key esta en este nodo
    bool found_in_node = (idx < node->count && node->keys[idx] == key);
    
    // 3: Key encontrada en nodo interno no hoja
    // Se reemplaza la key con su sucesor y luego eliminar recursivamente el sucesor
    if (found_in_node && !node->leaf) {
      TK sucesor = get_successor(node->children[idx + 1]);
      
      node->keys[idx] = sucesor;
      
      return remove_recursion(node->children[idx + 1], sucesor);
    }
    
    // 0: Key en nodo hoja 
    if (node->leaf) {
      // Si no esta en la hoja, no esta en el arbol
      if (!found_in_node) return false;
      
      // Eliminamos la key desplazando todas las keys siguientes una posición a la izquierda
      for (int i = idx; i < node->count - 1; i++) {
        node->keys[i] = node->keys[i + 1];
      }
      node->count--;
      return true;
    }
    
    // CASO 1 y 2: Key no esta en este nodo, se desciende al hijo apropiado
    Node<TK>* hijo = node->children[idx];
    
    bool encontrado = remove_recursion(hijo, key);
    
    if (!encontrado) return false;
    
    // Despues -> verificamos que el hijo no haya violado propiedades
    if (hijo->count < min_keys) {
      fix_children_remove(node, idx);
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
    
    int mid = M / 2;
    TK midKey = fullChild->keys[mid];
    
    // El hijo derecho recibe las keys después de mid
    newChild->count = fullChild->count - mid - 1;
    for (int i = 0; i < newChild->count; i++) {
        newChild->keys[i] = fullChild->keys[mid + 1 + i];
    }
    
    // Si no es hoja, también copiar los hijos correspondientes
    if (!fullChild->leaf) {
        for (int i = 0; i <= newChild->count; i++) {
            newChild->children[i] = fullChild->children[mid + 1 + i];
            fullChild->children[mid + 1 + i] = nullptr;
        }
    }
    
    // El hijo izquierdo mantiene las keys antes de mid
    fullChild->count = mid;
    
    // Insertar newChild en el array de children del padre
    for (int i = parent->count; i > childIndex; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[childIndex + 1] = newChild;
    
    // Insertar midKey en el array de keys del padre
    for (int i = parent->count - 1; i >= childIndex; i--) {
        parent->keys[i + 1] = parent->keys[i];
    }
    parent->keys[childIndex] = midKey;
    parent->count++;
  }

  // Helper para insertar en un subárbol y manejar el split si es necesario
  bool insertAndSplit(Node<TK>* node, TK key, TK& promotedKey, Node<TK>*& newSibling) {
    int i = node->count - 1;
    
    if (node->leaf) {
        // Insertar en hoja (permite temporalmente M keys)
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->count++;
        
        // Si el nodo ahora tiene M keys, necesita split
        if (node->count == M) {
            newSibling = new Node<TK>(M);
            newSibling->leaf = true;
            
            int mid = M / 2;
            promotedKey = node->keys[mid];
            
            // Copiar mitad derecha al nuevo hermano
            newSibling->count = node->count - mid - 1;
            for (int j = 0; j < newSibling->count; j++) {
                newSibling->keys[j] = node->keys[mid + 1 + j];
            }
            
            node->count = mid;
            return true;  // Indica que hubo split
        }
        return false;  // No hubo split
        
    } else {
        // Encontrar el hijo donde debe ir la key
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        
        TK childPromotedKey;
        Node<TK>* childNewSibling = nullptr;
        
        // Insertar recursivamente
        bool childDidSplit = insertAndSplit(node->children[i], key, childPromotedKey, childNewSibling);
        
        if (childDidSplit) {
            // El hijo hizo split, necesitamos insertar la key promovida en este nodo
            // Hacer espacio para la nueva key
            for (int j = node->count; j > i; j--) {
                node->keys[j] = node->keys[j - 1];
                node->children[j + 1] = node->children[j];
            }
            
            node->keys[i] = childPromotedKey;
            node->children[i + 1] = childNewSibling;
            node->count++;
            
            // Si ahora tenemos M keys, necesitamos split
            if (node->count == M) {
                newSibling = new Node<TK>(M);
                newSibling->leaf = false;
                
                int mid = M / 2;
                promotedKey = node->keys[mid];
                
                // Copiar mitad derecha
                newSibling->count = node->count - mid - 1;
                for (int j = 0; j < newSibling->count; j++) {
                    newSibling->keys[j] = node->keys[mid + 1 + j];
                }
                for (int j = 0; j <= newSibling->count; j++) {
                    newSibling->children[j] = node->children[mid + 1 + j];
                    node->children[mid + 1 + j] = nullptr;
                }
                
                node->count = mid;
                return true;
            }
        }
        
        return false;
    }
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

    TK promotedKey;
    Node<TK>* newSibling = nullptr;
    
    bool didSplit = insertAndSplit(root, key, promotedKey, newSibling);
    
    if (didSplit) {
        // La raíz hizo split, crear nueva raíz
        Node<TK>* newRoot = new Node<TK>(M);
        newRoot->leaf = false;
        newRoot->count = 1;
        newRoot->keys[0] = promotedKey;
        newRoot->children[0] = root;
        newRoot->children[1] = newSibling;
        root = newRoot;
    }
    
    n++;
  }

  // Función auxiliar para limpiar nodos vacíos recursivamente
  void cleanEmptyChildren(Node<TK>* nd) {
    if (nd == nullptr || nd->leaf) return;
    
    // Primero, limpiar recursivamente los hijos
    for (int i = 0; i <= nd->count; i++) {
      if (nd->children[i] != nullptr) {
        cleanEmptyChildren(nd->children[i]);
      }
    }
    
    // Luego, eliminar hijos vacíos de este nodo
    int writeIdx = 0;
    for (int readIdx = 0; readIdx <= nd->count; readIdx++) {
      if (nd->children[readIdx] != nullptr && nd->children[readIdx]->count > 0) {
        nd->children[writeIdx] = nd->children[readIdx];
        writeIdx++;
      } else if (nd->children[readIdx] != nullptr && nd->children[readIdx]->count == 0) {
        // Eliminar el hijo vacío
        delete nd->children[readIdx];
        nd->children[readIdx] = nullptr;
      }
    }
    
    // Limpiar los punteros restantes
    for (int i = writeIdx; i < M; i++) {
      nd->children[i] = nullptr;
    }
  }

  void remove(TK key){//elimina un elemento
    if (!root) return;
    bool found = remove_recursion(root, key);
    if (found) {
      n--;
      //si la raiz quedo vacia, pero este tiene un hijop, el hijo se convierte en la nueva raiz
      if (root->count == 0 && !root->leaf) {
        Node<TK>* old_rt = root;
        root = root->children[0];
        old_rt->children[0] = nullptr;
        delete old_rt;
      }

      // si el arbol esta totalmente vacio, eliminar la raiz
      if (root && root->count == 0 && root->leaf) {
        delete root;
        root = nullptr;
      }
    }
  };
  
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
    string result = toString(root, sep);
    // Eliminar el separador final si existe
    if (!result.empty() && result.size() >= sep.size()) {
        // Verificar si termina con el separador
        if (result.substr(result.size() - sep.size()) == sep) {
            result = result.substr(0, result.size() - sep.size());
        }
    }
    return result;
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
    
    std::cerr << "[CHECK] All properties OK" << std::endl;
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