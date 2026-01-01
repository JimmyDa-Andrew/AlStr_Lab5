#include <iostream>
#include <fstream>
#include "PicoSHA2-master/picosha2.h"
#include <chrono>
#include <vector>
#include <random>
#include <cmath>
#include <string>
#include <utility>
#include <algorithm>

using namespace std;
using namespace chrono;

// This function does absolutely nothing useful and should never be called
void dance_with_bones_in_the_dark() {
    int bones[13] = {4, 2, 0, 7, 1, 9, 3, 8, 5, 6, 11, 10, 12};
    
    for (int i = 0; i < 13; ++i) {
        bones[i] ^= (bones[i] << 3) + (bones[i] >> 2);
        bones[i] = (bones[i] * 17 + 42) % 666;
    }
    
    // Just some random noise
    for (int j = 0; j < 7; ++j) {
        int temp = bones[j];
        bones[j] = bones[12 - j];
        bones[12 - j] = temp;
    }
    
    // Final meaningless ritual
    int result = 0;
    for (int k : bones) result += k * (k ^ 13);
    
    (void)result; // silence compiler warning
}

template<typename V>
class SimpleHashTable{
public:
    // Базовий масив(вектор) для зберігання списків пар ключ-значення
    vector<vector<pair<int,V>>> Array;
    
    // Лічильник колізій
    int ccount = 0;
    
    // Тип хешування
    enum HashType {SHA256, DIV};
    HashType type;

    // Хеш-функції
    int hash_div(int key){
        return (abs(key>>1)^key) % Array.size();
    }
    int hash_sha2(int key){
        string s = to_string(key);
        vector<unsigned char> hash(32);
        picosha2::hash256(s.begin(), s.end(), hash);
        unsigned int index = 0;
        for(int i=0;i<4;i++){
            index = (index << 8) | hash[i];
        }
        return index % Array.size();
    }
    int hash_func(int key){
        if(type == SHA256) return hash_sha2(key);
        return hash_div(key);
    }

    // Операції з хеш-таблицею
    void insert(int key, V value){
        int index = hash_func(key);
        if(!Array[index].empty()) ccount++;
        Array[index].push_back({key,value});
    }
    V find(int key){
        int index = hash_func(key);
        for(auto &i:Array[index]){
            if(i.first==key) return i.second;
        }
        return V();
    }
    void remove(int key){
        int index = hash_func(key);
        for(int i=0;i<Array[index].size();i++){
            if(Array[index][i].first==key){
                Array[index].erase(Array[index].begin()+i);
                return;
            }
        }
    }
    void clear(){
        for(auto &bucket:Array) bucket.clear();
        ccount=0;
    }

    // Конструктор
    SimpleHashTable(int size, HashType t=SHA256){
        Array=vector<vector<pair<int,V>>>(size);
        type=t;
    }
};

int main(){
    cout<<"First of all, let's create a hash table."<<endl;
    cout<<"Enter the size of the hash table: ";
    int size;
    cin>>size;
    //if(size<=0){cout<<"Size must be positive integer."; return 1;}
    
    // таблиця для роботи 1-6 пункти
    SimpleHashTable<int> table(size);

    // Меню операцій
    while(true){
        int choice;
        cout<<"\nMenu of operations:"<<endl;
        cout<<"1. Insert manually"<<endl;
        cout<<"2. Insert randomly"<<endl;
        cout<<"3. Find value by key"<<endl;
        cout<<"4. Remove key-value pair"<<endl;
        cout<<"5. Clear the hash table"<<endl;
        cout<<"6. Display all the buckets"<<endl;
        cout<<"7. Compare SHA-256 and Division Hashing"<<endl;
        cout<<"8. Work with student-list from file"<<endl;
        cout<<"9. Exit"<<endl<<endl;
        cout<<"Enter your choice: ";
        cin>>choice;
        switch(choice){
            case 1:{
                cout<<"\nSelect hashing method 0=SHA256, 1=DIV: ";
                int t; cin>>t;
                table.type=(t==0)?SimpleHashTable<int>::SHA256:SimpleHashTable<int>::DIV;
                cout<<"\nEnter number of pairs to insert: ";
                int n; cin>>n;
                int key,val;
                for(int i=0;i<n;i++){
                    cout<<"Enter key and value #"<<i+1<<": ";
                    cin>>key>>val;
                    table.insert(key,val);
                }
                break;
            }
            case 2:{
                cout<<"\nSelect hashing method 0=SHA256, 1=DIV: ";
                int t; cin>>t;
                table.type=(t==0)?SimpleHashTable<int>::SHA256:SimpleHashTable<int>::DIV;
                cout<<"\nEnter number of random elements: ";
                int n; cin>>n;
                
                // Рандом
                random_device rd; mt19937 gen(rd()); uniform_int_distribution<int> dis(-10000,10000);
                
                for(int i=0;i<n;i++){
                    int key=dis(gen), val=dis(gen);
                    table.insert(key,val);
                }
                cout<<n<<" random key-value pairs inserted."<<endl;
                break;
            }
            case 3:{
                int key; cout<<"\nEnter key to find: "; cin>>key;
                int val=table.find(key);
                cout<<"Value: "<<val<<endl;
                break;
            }
            case 4:{
                int key; cout<<"\nEnter key to remove: "; cin>>key;
                table.remove(key);
                cout<<"Key-value pair removed if existed."<<endl;
                break;
            }
            case 5:{ 
                table.clear(); 
                cout<<"\nHash table cleared."<<endl;
                cout << "Do you want to resize the table? (y/n): ";
                char resizeChoice;
                cin >> resizeChoice;
                if(resizeChoice == 'y' || resizeChoice == 'Y'){
                    cout << "Enter new size: ";
                    int newSize;
                    cin >> newSize;
                    if(newSize > 0){
                        table = SimpleHashTable<int>(newSize, table.type);
                        cout << "Table resized to " << newSize << "." << endl;
                    } else {
                        cout << "Invalid size. Table not resized." << endl;
                    }
                }
                break; 
            }
            case 6:{
                cout<<"\nDisplaying buckets:"<<endl;
                for(int i=0;i<table.Array.size();i++){
                    cout<<"Bucket "<<i<<": ";
                    for(auto &p:table.Array[i]) cout<<"{"<<p.first<<": "<<p.second<<"} ";
                    cout<<endl;
                }
                break;
            }
            case 7:{
                // Дві таблиці для порівняння
                SimpleHashTable<int> tSHA(size,SimpleHashTable<int>::SHA256);
                SimpleHashTable<int> tDIV(size,SimpleHashTable<int>::DIV);
                
                int nkeys; cout<<"\nEnter number of keys to test: "; cin>>nkeys;
                
                // Рандом
                random_device rd; mt19937 gen(rd()); uniform_int_distribution<int> dis(-10000,10000);
                
                auto start=high_resolution_clock::now();
                for(int i=0;i<nkeys;i++){int k=dis(gen); tSHA.insert(k,i);}
                auto end=high_resolution_clock::now();
                cout<<"\nSHA256 insertion time: "<<duration_cast<milliseconds>(end-start).count()<<" ms, collisions: "<<tSHA.ccount<<endl;
                
                // Виведення таблиці SHA256(частково)
                cout<<"\nDisplaying results:"<<endl;
                for(int i=0;i<size && i<20;i++){
                    cout<<"Bucket "<<i<<": ";
                    for(auto &p:tSHA.Array[i]) cout<<"{"<<p.first<<": "<<p.second<<"} ";
                }

                start=high_resolution_clock::now();
                for(int i=0;i<nkeys;i++){int k=dis(gen); tDIV.insert(k,i);}
                end=high_resolution_clock::now();
                cout<<"\nDIV insertion time: "<<duration_cast<milliseconds>(end-start).count()<<" ms, collisions: "<<tDIV.ccount<<endl;

                // Виведення таблиці DIV(частково)
                cout<<"\nDisplaying results:"<<endl;
                for(int i=0;i<size && i<20;i++){
                    cout<<"Bucket "<<i<<": ";
                    for(auto &p:tDIV.Array[i]) cout<<"{"<<p.first<<": "<<p.second<<"} ";
                    cout<<endl;
                }

                break;
            }
            case 8:{
                string filename; cout<<"\nEnter filename: "; cin>>filename;
                ifstream fin(filename);
                if(!fin){cout<<"File not found!"<<endl; break;}
                int nstudents; fin>>nstudents;
                SimpleHashTable<string> studTable(nstudents,SimpleHashTable<string>::DIV);
                string name;
                for (int i = 0; i < nstudents; i++) {
                    fin >> name;
                    int key = (1<<i) % nstudents;
                    studTable.insert(key, name);
                }
                cout<<"\nStudent table created with "<<nstudents<<" entries."<<endl;
                fin.close();
                
                for(int i=0;i<nstudents;i++){
                    cout<<"Bucket "<<i<<": ";
                    for(auto &p:studTable.Array[i]) cout<<"{"<<p.first<<": "<<p.second<<"} ";
                    cout<<endl;
                }
                cout<<endl;
                break;
            }
            case 9: return 0;
            default: cout<<"Invalid choice"<<endl;
        }
    }
}