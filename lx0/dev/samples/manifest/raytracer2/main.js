function main()
{
    __lx_print("Hello World!");

    var document = engine.loadDocument("media2/appdata/tutorial_03/document.xml");
    document = null;
    
    __lx_print("Document released.");
}