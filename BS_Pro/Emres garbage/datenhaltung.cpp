//
// Created by Emre Cankaya on 11.05.2021.
//

 struct daten{
    char key;
    char value;
}

struct daten datenhaltung[n];

 int get(char* key,char* res){
    for(int i = 0; i < datenhaltung.size;i++){
        if( key == datenhaltung[i].key){
            return datenhaltung.value;
        }
    }
    return -1
}

int put (char* key,char* value){



    for(int i = 0; i < datenhaltung.size;i++){
        if(datenhaltung[i].key == key){
            datenhaltung[i].value = value;
            return 1;
        }

    }
    for(int z = 0; z < datenhaltung.size;z++){
        if(datenhaltung[i].key == 0){
            datenhaltung[i].key = key;
            datenhaltung[i].value = value;
            return 0;
        }
    }
    return -1;
}

int del(char* key){
     for(int i; i < datenhaltung.size){
         if(datenhaltung[i].key == key){
             datenhaltung.key = 0;
             datenhaltung.value = 0;
             return 1;
         }
     }
     return -1;
 }

