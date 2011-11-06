extern void doit();

void init2()
{

    doit();

}

void init()
{

    doit();
    memory_compare("a", "b", 1);
    init2();

}

