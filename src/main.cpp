//
// Created by zanejins on 2022/10/21.
//

#include<iostream>
#include <cstring>

const uint32_t COLUMN_USERNAME_SIZE =32;
const uint32_t COLUMN_EMAIL_SIZE = 255;

struct Row_t{
    uint32_t id;
    char* username= new char[COLUMN_USERNAME_SIZE+1];
    char* email = new char[COLUMN_EMAIL_SIZE+1];
};

typedef Row_t Row;

const uint32_t ID_SIZE = sizeof(((Row*)0)->id);
const uint32_t USERNAME_SIZE = sizeof(((Row*)0)->username);
const uint32_t EMAIL_SIZE = sizeof(((Row*)0)->email);
const uint32_t ID_OFFSET = 0;
const uint32_t USER_OFFSET = ID_OFFSET+ID_SIZE;
const uint32_t EMAIL_OFFSET = USER_OFFSET+USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE+USERNAME_SIZE+EMAIL_SIZE;


const uint32_t PAGE_SIZE = 4096;
const uint32_t TABLE_MAX_PAGES = 100;
const uint32_t ROW_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS= TABLE_MAX_PAGES * ROW_PER_PAGE;

struct Table_t {
    void* pages[TABLE_MAX_PAGES];  // one table -> many pages
    uint32_t num_rows;
};
typedef struct Table_t Table;

void print_row(Row* row) {
    std::cout<<row->id<<" "<< row->username<<" "<<row->email<<std::endl;
}


enum MetaCommandResult_t {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECONGNIZED_COMMAND
};
typedef enum MetaCommandResult_t MetaCommandResult;

enum PrepareResult_t {
    PREPARE_SUCCESS,
    PREPARE_NEGATIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
};
typedef PrepareResult_t PrepareResult;

enum StatementType_t {
    STATEMENT_INSERT,
    STATEMENT_SELECT
};
typedef enum StatementType_t StatementType;

enum ExecuteResult_t {
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
};

typedef ExecuteResult_t ExecuteResult;

struct Statement_t {
    StatementType type;
    Row row_to_insert; //only used by insert statement
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
PrepareResult prepare_insert(InputBuffer* input_buffer,Statement* statement) {
    statement->type=STATEMENT_INSERT;
    //you can use sccanf to achive this function, but sccanf has some disadvantages.
    // If the string it's reading is larger than the buffer it's reading into, it will cause a buffer overstack
    // and writing into unexpected places. We want to check the length of each string before we copy it into
    // a row structure. And to do that, we need to divide the input by space.
    // So here we use strtok() to do that, I think it is easiest to understand if you see it in action.
    char* keyword = strtok(input_buffer->buffer," ");
    char* id_string = strtok(NULL," ");
    char* username = strtok(NULL," ");
    char* email = strtok(NULL," ");

    if(id_string ==NULL || username ==NULL || email ==NULL) {
        return PREPARE_SYNTAX_ERROR;
    }

    int id=atoi(id_string);
    if(id<0) {
        return PREPARE_NEGATIVE_ID;
    }
    if(strlen(username)>COLUMN_USERNAME_SIZE || strlen(email)>COLUMN_EMAIL_SIZE) {
        return PREPARE_STRING_TOO_LONG;
    }

    statement->row_to_insert.id=id;
    statement->row_to_insert.username=username;
    statement->row_to_insert.email=email;
    return PREPARE_SUCCESS;
}
PrepareResult prepare_statement(InputBuffer* input_buffer,Statement* statement) {
    if(strncmp(input_buffer->buffer,"insert",6)==0) {
        return prepare_insert(input_buffer,statement);
    }

    if(strncmp(input_buffer->buffer,"select",6)==0) {
        statement->type=STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }

    return PREPARE_UNRECOGNIZED_STATEMENT;
}
void serialize_row(Row* source,void* destination) {
    memcpy(destination + ID_OFFSET,&(source->id),ID_SIZE);
    memcpy(destination+USER_OFFSET,&(source->username),USERNAME_SIZE);
    memcpy(destination+EMAIL_OFFSET,&(source->email),EMAIL_SIZE);
}
void* row_slot(Table* table,uint32_t row_num) {
    uint32_t page_num = row_num / ROW_PER_PAGE;
    void* page = table -> pages[page_num];
    if(!page) {
        //allocate memory only when we try to access page
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROW_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page+byte_offset;
}

void desciralize_row(void* source,Row* destination) {
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USER_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}
ExecuteResult execute_insert(Statement* statement,Table* table) {
    if(table->num_rows>=TABLE_MAX_ROWS) {
        return EXECUTE_TABLE_FULL;
    }

    Row* row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert,row_slot(table,table->num_rows));
    table->num_rows++;
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Statement* statement,Table* table) {
    Row row;
    for(uint32_t i=0;i<table->num_rows;i++) {
        desciralize_row(row_slot(table,i),&row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}
ExecuteResult execute_statement(Statement* statement,Table* table) {
    switch (statement->type) {
        case(STATEMENT_INSERT):
            return execute_insert(statement,table);
        case(STATEMENT_SELECT):
            return execute_select(statement,table);
    }
}

Table* new_table() {
    Table* table = (Table*)malloc(sizeof(Table));
    table->num_rows=0;
    return table;
}
int main() {
    Table* table = new_table();
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
        switch(prepare_statement(input_buffer,&statement)) {
            case(PREPARE_SUCCESS):
                break;
            case(PREPARE_NEGATIVE_ID):
                cout<<"ID must be positive."<<endl;
                continue;
            case(PREPARE_STRING_TOO_LONG):
                cout<<"String is too long"<<endl;
                continue;
            case(PREPARE_SYNTAX_ERROR):
                cout<<"Syntax error.Could not parse statement."<<endl;
                continue;
            case(PREPARE_UNRECOGNIZED_STATEMENT):
                cout<<"Unrecognized keyword at start of "<<input_buffer->buffer<<endl;
                continue;
        }
        switch(execute_statement(&statement,table)) {
            case(EXECUTE_SUCCESS):
                cout<<"Executed."<<endl;
                break;
            case(EXECUTE_TABLE_FULL):
                cout<<"The table is full"<<endl;
                break;
        }
    }
}