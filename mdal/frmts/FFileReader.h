// Module developped by herve ozdoba - Sept 2008 ( herve-externe.ozdoba at edf.fr / herve at ozdoba.fr )

#ifndef __FFileReader_h__
#define __FFileReader_h__

/** -- Inclusions issues de la bibliothèque standard du C++ -- */

#include <fstream>
#include <string>
#include <cstdio>
#include <iostream>
#include <cstring>
#include <stdint.h>   // need 64bit file position variables

using namespace std;

/** ********************************************************************************************************* **/
/** -- Definition de la classe abstraite de lecture des fichiers issus du langage de programmation Fortran -- **/
/** ********************************************************************************************************* **/

/// Cette classe, destinée uniquement à lire un fichier Serafin écrit par Telemac sous Fortran, permet de simplifier la lecture
/// des données binaires en prenant en charge les systèmes d'écriture big/little endian spécifiques à certaine architecture .

// Passe de quatre octets à la lecture d'un flux
#define skipReadingHeader(stream) (stream->seekg (sizeof(int), ios_base::cur ))

class FFileReader
{
public:

    // Simple constructeur (flux en argument)
    FFileReader(ifstream* stream);

    // Destructeur de base
    ~FFileReader();	

    // [inline] Retourne true si le fichier est écrit en big endian, cette fonction utilise la taille maximum du titre de la simu pour
    // déterminer cette propriété (donc cette classe ne peut en aucun être utilisée hors de son domaine actuel d'utilisation)
    // TODO Modifier le nom de cette méthode, elle n'indique pas le système d'écriture mais uniquement la nécessité d'inverser ou non l'ordre des bytes à
    // la lecture du fichier .
    bool IsBigEndian(){return this->BigEndian;};
        
    // [inline] Déplace le pointeur de lecture du fichier vers le bloc d'écriture suivant dans le fichier fortran et retourne la position actuelle .
    int GoToNextBloc()
    {
        FileStream->seekg (GetBlocSize() + 2*sizeof(int), ios_base::cur);
        if (IsBigEndian()) s_readBlocSize ();else  ns_readBlocSize ();

        return FileStream->tellg();
    };

    // [inline] Retourne la taille du bloc d'écriture actuel .
    int GetBlocSize(){return this->BlocSize;};
        // float size - allow for 32 or 64 bit floats
    
    // Lit et stocke des tableaux sous différents formats (la taille est spécifiée en second argument)
        // these functions return a file position
    int64_t	  (FFileReader:: *readIntArray)    (int* , const int); 
    int64_t	  (FFileReader:: *readFloatArray)  (float*, const int);
    
    // Quelques macros pour simplification d'écriture sur pointeur de fonction
    // TODO Redefinir les macros, elle ne sont plus valables ...	à dédéfinir du type '(*this.*readFloatArray)' pour utilisation ultérieure .
    #define ReadIntArray (*readIntArray)
    #define ReadFloatArray (*readFloatArray)
    /*#define ReadStringArray (*ReadStringArray)*/
    
    // [inline] Lit une chaîne de caractères en fonction de la taille passée en argument et se déplace sur le bloc suivant
    int64_t	  ReadString(char* s, int size)
    {
        skipReadingHeader(FileStream);
                FileStream->read  (s, size);
                skipReadingHeader(FileStream);
        readBlocSize ();
        // if (IsBigEndian()) s_readBlocSize ();else  ns_readBlocSize ();
                
        return FileStream->tellg();
    };
    
    // lecture de tableaux avec inversion des octets ou non 	
    int64_t s_readInt32Array(int* arr, const int size);
    int64_t ns_readInt32Array(int* arr, const int size);
    int64_t g_readInt32Array(int* arr, const int size);

    int64_t s_readFloat32Array(float* arr, const int size);
    int64_t ns_readFloat32Array(float* arr, const int size);
    int64_t g_readFloat32Array(float* arr, const int size);
    
    // Retourne la taille du fichier 
    // TODO A placer en protected par la suite
    int64_t GetFileSize()
    {
        // sauvegarder la position courante
        int64_t pos = FileStream->tellg();
        
        // se placer en fin de fichier
        FileStream->seekg( 0 , std::ios_base::end );
        
        // récupérer la nouvelle position = la taille du fichier
        int64_t size = FileStream->tellg() ;
        
        // restaurer la position initiale du fichier
        FileStream->seekg( pos,  std::ios_base::beg ) ;
        
        return size ;
    };


protected:

    FFileReader(); // Non-implémentée
    
    bool BigEndian; // Système d'écriture du fichier
    int  BlocSize;  // Taille du bloc d'écriture suivant
    int  FloatSize;

    ifstream *FileStream; // Le flux d'entrée du fichier

    // [inline] Lecture d'un entête avec ou sans swap (indicateur par préfixe)
    void s_readBlocSize () {ns_readBlocSize (); Swap32((char*)(&BlocSize));};
    void ns_readBlocSize () {FileStream->read ((char*)(&BlocSize), sizeof(int));FileStream->seekg ( -sizeof(int), ios_base::cur );};
        
    void readBlocSize()
        {
            FileStream->read ((char*)(&BlocSize), sizeof(int));
            // always reposition it back to its start .....
            FileStream->seekg ( -sizeof(int), ios_base::cur );
            if ( this->BigEndian) {
                Swap32((char*)(&BlocSize));
            }
            cerr << "BlocSize " << BlocSize << "\n";
                
        };

private:
    //[inline] Gestion des swaps pour la prise en charge l/ge
    #define Intervert(i,j) {one_byte = data[i]; data[i] = data[j]; data[j] = one_byte;}
    void Swap32	 (char* data) {char one_byte;Intervert(0,3);Intervert(1,2);}
    void Swap32Array (const long int size, char* data) {long int indent;for(indent = 0; indent!= size; indent++) Swap32(&data[indent*4]);};
    
    FFileReader(const FFileReader&); // Pas implémentée
    void operator=(const FFileReader&);   // Pas implémentée

}; /* class_FFileReader */

#endif
