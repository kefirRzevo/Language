#include "../include/tree.h"

static const char* LOG_PATH = "logfiles/dump";
static const char* DOT_PATH = "logfiles/res/tree";
static size_t N_DUMPS       = 0;
static FILE*  LOG           = nullptr;

static FILE*  dot_init(const char*  dot_path);
static FILE* dump_init(const char* dump_path);
static void   dot_close(FILE* const dotfile);
static void  dump_close();
static void  create_dump(Tree* p_tree, char* img_path);
static void  print_nodes(Node* node, FILE* const dotfile);
static void  write_nodes(Node* node, FILE* const file);
static void  make_one_dump(Tree* p_tree, char* img_path);
static const char* oper_name(Node* node);

#define ass(X)  if(!X)  return nullptr

static FILE* dot_init(const char*  dot_path)
{
    assert(dot_path);

    FILE* dotfile   = nullptr;
    dotfile         = fopen(dot_path, "w");
    ass(dotfile);
    
    fprintf(dotfile,  "\tdigraph\n"
                      "\t{\n"
                      "\t\tedge [color     = darkgrey]\n"
                      "\t\tedge [arrowsize = 1.6]\n"
                      "\t\tedge [penwidth  = 1.2]\n"
                      "\n"
                      "\t\tgraph [penwidth = 2]\n" 
                      "\t\tgraph [ranksep  = 1.3]\n" 
                      "\t\tgraph [nodesep  = 0.5]\n"
                      "\t\tgraph [style    = \"filled\"]\n"
                      "\t\tgraph [color    = green, fillcolor = lightgreen]\n"
                      "\n"
                      "\t\tnode [penwidth = 2]\n" 
                      "\t\tnode [shape    = box]\n" 
                      "\t\tnode [color    = black, fillcolor = white]\n"
                      "\n"
                      "\t\tcompound  = true;\n"
                      "\t\tnewrank   = true;\n\n");
    return dotfile;
}

static void dot_close(FILE* const dotfile)
{
    assert(dotfile);

    fprintf(dotfile, "\t}");
    fclose(dotfile);
}

static FILE* dump_init(const char* const dump_path)
{
    assert(dump_path);

    if(!LOG)
    {
        LOG = fopen(dump_path, "w");
        ass(LOG);

        fprintf(LOG,    "<!doctype html>                                                             \n"
                        "<html>                                                                      \n"
                        "\t<head lang='en'>                                                          \n"
                        "\t\t<meta charset='utf-8'>                                                  \n"
                        "\t\t<meta name='viewport' content='width=device-width'>                     \n"
                        "\t\t<title>Tree dump</title>                                                \n"
                        "\t\t<style>                                                                 \n"
                        "\t\t.tree                                                                   \n"
                        "\t\t{                                                                       \n"
                        "\t\t\twidth: max-content;                                                   \n"
                        "\t\t\tfont-size: 20px;                                                      \n"
                        "\t\t\tpadding: 10px 20px;                                                   \n"
                        "\t\t}                                                                       \n"
                        "\t\t.img                                                                    \n"
                        "\t\t{                                                                       \n"
                        "\t\t\toverflow-x: auto;                                                     \n"
                        "\t\t}                                                                       \n"
                        "\t\t</style>                                                                \n"
                        "\t</head>                                                                   \n"
                        "\t<body>");
    }
    return LOG;
}

static void dump_close()
{
    fprintf(LOG, "\n\t</body>\n"
                     "</html>\n");
}

Tree* tree_dump(Tree* p_tree)
{
    assert(p_tree);

    char cmd_string[64]     = "";
    char dot_path[32] = "";
    char dump_path[32]      = "";
    char img_path[32]       = "";


    sprintf(dot_path,   "%s%zu.dot",            DOT_PATH, N_DUMPS);
    sprintf(img_path,   "%s_img%zu.svg",        DOT_PATH, N_DUMPS);
    sprintf(cmd_string, "dot %s -Tsvg -o %s",   dot_path, img_path);
    sprintf(dump_path,  "%s.html",              LOG_PATH);


    FILE* dotfile = dot_init(dot_path);
    ass(dotfile);
    print_nodes(p_tree->root, dotfile);
    dot_close(dotfile);
    system(cmd_string);

    LOG = dump_init(dump_path);
    make_one_dump(p_tree, img_path);
    if(!N_DUMPS)
        atexit(dump_close);
    N_DUMPS++;
    return p_tree;
}

static void make_one_dump(Tree* p_tree, char* img_path)
{
    fprintf(LOG,    "\n\t\t<hr width = '100%%'>\n"
                        "\t\t<div class = 'list'>\n"
                        "\t\t\t<pre>DUMP %zu</pre>\n"
                        "\t\t\t<pre>List address: %p</pre>\n"
                        "\t\t\t<pre>Size:         %zu</pre>\n"
                        "\t\t</div>\n"
                        "\t\t<div class = 'img'>\n"
                        "\t\t\t<img src = \"../%s\" height = 500px>\n"
                        "\t\t</div>", N_DUMPS + 1, p_tree, p_tree->size, img_path);
}

static const char* oper_name(Node* node)
{
    assert(node->Type == OPER);

    #define OPER(name, keyword, ident)                          \
                    else if (keyword == node->Value.oper_type)  \
                        return ident;                      

                    if (0) {} 
    #include "../STANDARD_TREE"
    #undef OPER

    return nullptr;
}

static void print_nodes(Node* node, FILE* const dotfile)
{
    assert(dotfile);
    assert(node);

    switch(node->Type)
    {
        case NUM:   fprintf(dotfile, "\t\tNode%p[label = \"%" NUM_SPEC  "\"shape = oval,    color = yellow]\n", node, node->Value.number);
                    break;
        case OPER:  fprintf(dotfile, "\t\tNode%p[label = \"%" OPER_SPEC "\"shape = diamond, color = red]\n",    node, oper_name(node));
                    break;
        case ID:    fprintf(dotfile, "\t\tNode%p[label = \"%"  ID_SPEC  "\"shape = octagon, color = blue]\n",   node, node->Value.ident);
                    break;
        default:    fprintf(stderr, "Unknown error, %d.", __LINE__); 
                    break;
    }

    if(node->Left)
    {
        fprintf(dotfile,    "\t\tNode%p -> Node%p\n", node, node->Left);
        fprintf(dotfile, "\n");
        print_nodes(node->Left,  dotfile);
    }
    if(node->Right)
    {
        fprintf(dotfile,    "\t\tNode%p -> Node%p\n", node, node->Right);
        fprintf(dotfile, "\n");
        print_nodes(node->Right, dotfile);
    }
}

static void write_nodes(Node* node, FILE* const file)
{
    assert(file);
    
    fprintf(file, "(");

    if(node->Left)
        write_nodes(node->Left, file);

    switch(node->Type)
    {
        case NUM:   fprintf(file, "%"  NUM_SPEC "",  node->Value.number);
                    break;
        case OPER:  fprintf(file, "%" OPER_SPEC "",  oper_name(node));
                    break;
        case ID:    fprintf(file, "'%"  ID_SPEC "'", node->Value.ident);
                    break;
        default:    fprintf(stderr, "Unknown error, %d.", __LINE__); 
                    break;
    }

    if(node->Right)
        write_nodes(node->Right, file);
    
    fprintf(file, ")");
}

Tree* tree_write(Tree* p_tree, const char* filepath)
{
    FILE* fp = fopen(filepath, "w");
    if(!fp)
        return nullptr;

    write_nodes(p_tree->root, fp);
    fclose(fp);

    return p_tree;
}
