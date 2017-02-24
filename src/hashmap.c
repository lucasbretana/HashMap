#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hashmap.h"
#include "hashmapJon.h"
#include "usefull.h"
#include "linkedList.h"
/*
  Generates the hash code of a key
  Note that this do not return the hash map position
 */
h_code_t h0(void *k, int len){
  char *p = (char *) k;
  h_code_t hash = 2166136261L;
  for(int i=1; i<len-1; i++){
    hash = hash * 16777619;  	// Multiply by prime number found to work well
    hash = hash ^ (p[i]); 	// xor next byte into the bottom of the hash
  }
  return llabs(hash);
}

/**
 * Calculates the position in the given hash for the given key
 * Also puts the generated code in the code pointer
 */
position_t h1(key_p k, bulk_t size, h_code_t *code){
  *code = h0(k, length(k));
  return (*code) % size;
}

/**
 * Calculates the position in the given hash for the given key, to be used
 * in the double hash
 */
position_t h2(key_p k, bulk_t size){
  return 1 + ( h0(k, length(k)) % (size - 1) );
}

/**
 * If it is Chaining, then to remove it must find in the list
 * If it is Double_Hash then is start to look in the hash list using the double
 * hash deslocation, stop when you found or when you run size of hash times
 * If it is Double_Hash then is start to look in the hash list using the
 * quadratic deslocation, stop when you found or when you run size of hash times
 * If is is Linear then is start to look in the hash list, stop when you found
 * it or look where it started
 */
ReturnLog_t hash_delete(HashMap_t *hash, key_p hashKey){
  ReturnLog_t log;
  char *string = NULL;
  hashList *list;
  // h_code_t code;

  switch(hash->method){
    case Chaining:
      log.indH1 = h1(hashKey, length(hashKey), &log.code);
      log.indHash = log.indH1;
      list = ((hashList *)hash->keys) + log.indH1;
      log.localConflicts = list_delete(&list, hashKey);
      log.success = TRUE;
      if(log.localConflicts == -1){
        log.localConflicts = 1;
        log.success = FALSE;
      }
      break;
    case Double_Hash:
      log = hash_get(hash, hashKey);
      string = *(((key_p *)hash->keys) + log.indHash);
      *(((key_p *)hash->keys) + log.indHash) = NULL;
      free(string);
      break;
    case Quadratic:
      log = hash_get(hash, hashKey);
      string = *(((key_p *)hash->keys) + log.indHash);
      *(((key_p *)hash->keys) + log.indHash) = NULL;
      free(string);
      break;
    case Linear:
      log = hash_get(hash, hashKey);
      string = *(((key_p *)hash->keys) + log.indHash);
      *(((key_p *)hash->keys) + log.indHash) = NULL;
      free(string);
      break;
    default:
      fprintf(stderr, "There was something wrong! The conflict methodis not valid!\n");
      break;
    }
    if(log.success == TRUE) hash->nEntrys--;
    return log;
}

ReturnLog_t hash_get(HashMap_t *hash, key_p hashKey){
  ReturnLog_t log;
  hashList *list;
  switch(hash->method){
    case Chaining:
      {
        log.indH1 = h1(hashKey, length(hashKey), &log.code);
        log.indHash = log.indH1;
        list = ((hashList *)hash->keys) + log.indH1;
        if(list_get(list, hashKey) == NULL){
          log.localConflicts = 1;
          log.success = FALSE;
        }else if(list_get(list, hashKey)->prev == NULL){
          log.localConflicts = 0;
          log.success = TRUE;
        }else{
          log.localConflicts = 1;
          log.success = TRUE;
        }
      }
      break;
    case Double_Hash:
      {
        // h'(k, i) = (h1(k) + i * h2(k)) mod size
        log.indH1 = h1(hashKey, hash->size, &log.code);
        unsigned int indH2 = h2(hashKey, hash->size);
        log.indHash = log.indH1;
        log.localConflicts = 0;
        log.success = FALSE;
        unsigned i = 0;
        // for(unsigned i = 0; i <hash->size ; i++){
        // log.indHash = (log.indHash + i * indH2) % hash->size;
        do{
          char *string;
          string = *(((key_p *)hash->keys) + log.indHash);
          if(strcomp(string, hashKey) == 0){
            log.success = TRUE;
            // *(((key_p *)hash->keys) + log.indHash) = NULL;
            // free(string);
            break;
          }else{
            log.localConflicts++;
          }
          i++;
          log.indHash = (log.indHash + i * indH2) % hash->size;
        }while(log.indHash != log.indH1);
        // }
      }
      break;
    case Quadratic:
      {
        // h'(k, i) = (h1(k) +i^2) mod size
        log.indH1 = h1(hashKey, hash->size, &log.code);
        log.indHash = log.indH1;
        log.localConflicts = 0;
        log.success = FALSE;
        // for(unsigned i = 0; i <hash->size ; i++){
        unsigned i = 0;
        do{
          // log.indHash = (log.indHash + i*i) % hash->size;
          char *string;
          string = *(((key_p *)hash->keys) + log.indHash);
          if(strcomp(string, hashKey) == 0){
            log.success = TRUE;
            // *(((key_p *)hash->keys) + log.indHash) = NULL;
            // free(string);
            break;
          }else{
            log.localConflicts++;
          }
          i++;
          log.indHash = (log.indHash + i + i*i) % hash->size;
        }while(log.indHash != log.indH1);
        // }
      }
      break;
    case Linear:
      {
        // On this case what we have is a char ** || key_p *
        // h'(k, i) = (h1(k) +i) mod size
        log.indH1 = h1(hashKey, hash->size, &log.code);
        log.indHash = log.indH1;
        log.localConflicts = 0;
        log.success = FALSE;
        for(unsigned i = 0; i <hash->size ; i++){
          log.indHash = (log.indHash + i) % hash->size;
          char *string;
          string = *(((key_p *)hash->keys) + log.indHash);
          if(strcomp(string, hashKey) == 0){
            log.success = TRUE;
            // *(((key_p *)hash->keys) + log.indHash) = NULL;
            // free(string);
            break;
          }else{
            log.localConflicts++;
          }
        }
      }
      break;
    default:
      fprintf(stderr, "There was something wrong! The conflict methodis not valid!\n");
      break;
    }

  return log;
}

HashMap_t *rehash(HashMap_t *hash){
  // fprintf(stderr, "Before: %p\t", hash);
  HashMap_t *newHash = hash_initialize(hash->method, hash->size * 2);
  // int indOldHash = 0, indNewHash = 0;

  if(hash->method != Chaining){
    for(unsigned i = 0; i <hash->size ; i++){
      char *string;
      string = *(((key_p *)hash->keys) + i);
      if(string != NULL){
        hash_insert(&newHash, string);
      }
    }
  }else{
    fprintf(stderr, "\nBegin if else");
    hashList *seeker;
    for(unsigned i = 0; i <hash->size ; i++){
      fprintf(stderr, "\nFor loop %d", i);
      seeker = *(((hashList**)hash->keys) + i);
      if(seeker->data != NULL){
        hash_insert(&newHash, seeker->data);
      }
    //  list_free(seeker);
    }
  }
  fprintf(stderr, "\nEnd else");
  // printf("\nBefore hash_free");
  fprintf(stderr, "\nEnd else1");
  hash_free(hash);
  fprintf(stderr, "\nAfter hash_free\n");
  return newHash;
}

void hash_free(HashMap_t *hash){
  if(hash == NULL) return;
  if(hash->method != Chaining){
    for(unsigned i=0 ; i<hash->size ; i++){
      if(*(((key_t **)hash->keys) + i) != NULL) free(*(((key_t **)hash->keys) + i));
      *(((key_t **)hash->keys) + i) = NULL;
      // if((((key_t **)hash->keys) + i) != NULL) free((((key_t **)hash->keys) + i));
    }
  }else{
    for(unsigned i=0 ; i<hash->size ; i++){
      list_free(*(((hashList **)hash->keys) + i));
      // if(*(((hashList **)hash->keys) + i) != NULL) free(*(((hashList **)hash->keys) + i));
      *(((hashList **)hash->keys) + i) = NULL;
      // if((((hashList **)hash->keys) + i) != NULL) free((((hashList **)hash->keys) + i));
    }
  }
  printf("free(hash->keys)");
  free(hash->keys);
}
