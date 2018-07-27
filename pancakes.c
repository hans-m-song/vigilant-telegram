#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define BUFF 10

enum err{OK, E_IO, E_MEM, E_STACK};

typedef struct Pancakes{
    int size;
    int *stack;
    int *out;
    int out_size;
    int active;
    int flips;
    FILE *f;
} Pancakes;

// cleans up pancake instance
void de_init(Pancakes *p){
    free(p->out);
    free(p->stack);
    if(p->f != stdin){
        fclose(p->f);
    }
}

// shows pancake stacks current state
// mode: v for visual, t for text
// takes an array of integers and the size to visualize
// outputs to the given output stream
// returns an error code
int visualize(char mode, int *stack, int size, FILE *f){
    int i;
    for(i=0;i<size;i++){
        int len = abs(stack[i]);
        if(mode == 't') {
            fprintf(f, "%d\n", stack[i]);
        } else if(mode == 'v') {
            char *output = malloc(sizeof(char)*abs(stack[i]+1));
            output[abs(stack[i])] = '\0';
            if(stack[i] > 0) {
                memset(output, '^', abs(stack[i]));
            } else {
                memset(output, 'v', abs(stack[i]));
            }
            fprintf(f, "%s\n", output);
            free(output);
        }
    }
    fprintf(f, "\n");
    fflush(f);
    return OK;
}

// initalize pancakes instance and processes input
// takes a pointer to an instance of pancakes and an input stream
// returns an error code
int init(Pancakes *p, FILE *f){
    // get number of pancakes
    char stack_size[BUFF];
    if(!fgets(stack_size, BUFF, f)){
        printf("get size err\n");
        return E_IO;
    }
    p->size = atoi(stack_size);
    printf("expected pancakes: %d\n", p->size);

    p->active = p->size;
    p->flips = 0;
    p->f = f;
    
    p->stack = malloc(sizeof(int)*p->size);
    if (!p->stack){
        return E_MEM;
    }

    int i;
    for(i=0;i<p->size;i++){
        char temp[BUFF];
        p->stack[i] = atoi(fgets(temp, BUFF, p->f));
        if (!temp){
            return E_IO;
        }
    }
    printf("received pancakes:\n");
    visualize('t', p->stack, p->size, stdout);
    //visualize('v', p->stack, p->size, stdout);

    p->out_size = p->size*2;
    p->out = malloc(sizeof(int)*p->out_size);

    return OK;
}

// returns index of max value of given stack
// takes an array of integers and the number of elements to look at
// returns the index of the biggest element
int max(int *stack, int active){
    int i = 0;
    int j = 0;
    int max = 0;
    for(i = 0; i < active; i++){
        if(abs(stack[i]) > max){
            max = abs(stack[i]);
            j = i;
        }
    }
    printf("maximum is %d at %d\n", max, j);
    return j;
}

// flips the array
// takes an array of integers and the number of elements to look at
int *flip(int *stack, int active){
    //printf("flipping %d pancakes: ", active);
    //visualize('t', stack, active, stdout);
    int i = 0;
    int j = active-1;
    int temp = 0;
    while(i<j){    
        //printf("flipping pancakes %d:%d and %d:%d\n", i, stack[i], j, stack[j]);
        temp = stack[i];
        stack[i] = stack[j];
        stack[j] = temp;
        i++;
        j--;
    }
    int k;
    for(k=0;k<active;k++){
        stack[k] *= -1;
    }
    //printf("flipped: ");
    //visualize('t', stack, active, stdout);
    return stack;
}

// checks if top pancake needs to be flipped
// takes an integer representing a pancake
// returns 1 if flipping required
int oriented(int pancake){
    if(pancake < 0){
        return 1;
    }
    return 0;
}

// invocation: ./pancake [input file]
int main(int argc, char** argv){
    int err = 0;
    
    // check for user specified file
    FILE *f;
    if(argc > 1){
        f = fopen(argv[1], "r");
        if(!f){
            return E_IO;
        }
    } else {
        f = stdin;
    }

    struct Pancakes p;
    init(&p, f);

    while(p.active > 0){
        // find biggest pancake and save for flip
        p.out[p.flips] = max(p.stack, p.active); 
        p.stack = flip(p.stack, p.out[p.flips++]+1);
        if(!p.stack){
            return E_STACK;
        }
        
        // check for correct orientation of topmost pancake
        if (!oriented(p.stack[0])){ 
            p.stack[0] *= -1;
            p.out[p.flips++] = 0;
        }
    
        // flip active stack to move biggest to bottom
        p.out[p.flips] = p.active-1;
        p.stack = flip(p.stack, p.active--); // exclude sorted pancake
        
        if(p.flips > p.out_size-3){ // check if output array needs more memory
            p.out_size *= 2;
            p.out = realloc(p.out, sizeof(int)*p.out_size);
        }

        //printf("stack after %d iterations is:\n", p.size-p.active);
        //visualize('v', p.stack, p.size, stdout);
    }

    printf("flips required at: ");
    visualize('t', p.out, p.flips, stdout);
    
    //printf("final stack:\n");
    //visualize('v', p.stack, p.size, stdout);

    // clean up for exit
    de_init(&p);
    return OK;
}
