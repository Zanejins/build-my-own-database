//
// Created by zanejins on 2022/10/21.
//

#include<iostream>
#include <cstring>

enum MetaCommandResult_t {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECONGNIZED_COMMAND
};
typedef enum MetaCommandResult_t MetaCommandResult;

enum PrepareResult_t {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT
};
typedef PrepareResult_t PrepareResult;

enum StatementType_t {
    STATEMENT_INSERT,
    STATEMENT_SELECT
};
typedef enum StatementType_t StatementType;

struct Statement_t {
    StatementType type;
};

typedef struct Statement_t Statement;

using namespace std;
const int MaxBufferSize=100;
typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
}InputBuffer;

InputBuffer* new_input_buffer() {
    InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    input_buffer->buffer = new char[MaxBufferSize];
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void print_prompt(){
    cout<<"db> ";
}

void read_input(InputBuffer* input_buffer) {
    string s;
    getline(cin,s);
    strcpy(input_buffer->buffer,s.c_str());
    input_buffer->input_length=strlen(input_buffer->buffer);
}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}
MetaCommandResult do_meta_command(InputBuffer* input_buffer) {
    if(strcmp(input_buffer->buffer,".exit")==0) {
        exit(EXIT_SUCCESS);
    }
    else {
        return META_COMMAND_UNRECONGNIZED_COMMAND;
    }
}
PrepareResult prepare_statement(InputBuffer* input_buffer);
int main() {
    setbuf(stdout,NULL);
    InputBuffer* input_buffer = new_input_buffer();
    while(1) {
        print_prompt();
        read_input(input_buffer);

        if(input_buffer->buffer[0]=='.') {
            switch(do_meta_command(input_buffer)) {
                case(META_COMMAND_SUCCESS):
                    continue;
                case(META_COMMAND_UNRECONGNIZED_COMMAND):
                    cout<<"Unrecognized command "<<input_buffer->buffer<<endl;
                    continue;
            }
        }

        Statement statement;

    }
}