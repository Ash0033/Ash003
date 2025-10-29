//Յուրաքանչյուր node ունի երկու հղում DoublyLinkedListNode
DoublyLinkedListNode* newNode = new DoublyLinkedListNode(data);// Ստեղծում ենք նոր node
  //Եթե ցանկը դատարկ է, նոր node-ը դառնում է head
    if (!head)  
        return newNode;

    // Եթե պետք է ավելացնենք առաջինի առաջ
    if (data <= head->data) //Եթե տվյալը փոքր է head-ի կամ հավասար, դնում ենք սկզբում
    { 
        newNode->next = head;
        head->prev = newNode; //նախորդ
        return newNode;
    }

    DoublyLinkedListNode* current = head;

    // Գնալ ճիշտ տեղը
     //գոյություն ունի հաջորդ node   հաջորդ node-ի արժեքը փոքր է նոր node-ի արժեքից
    while (current->next && current->next->data < data) 
    {
        current = current->next;
    } //Երբ ցիկլը կանգ է առնում՝ current արդեն այն node-ն է, որից հետո պետք է նոր node-ը դնել

    // Կապել նոր node-ը
    newNode->next = current->next;
    newNode->prev = current;

    if (current->next) {
        current->next->prev = newNode;
    }
//Եթե մենք չգրեինք սա, այդ node-ի prev-ը կխառնվեր, doubly linked list-ը կվնասվեր
    
    current->next = newNode;
    return head;
//Այս դեպքում head-ը չի փոխվել, որովհետև մենք նոր node չենք դնել սկզբում։
//Եթե նոր node-ը սկզբում լիներ, մեր if-ով վերադարձնելու էինք նոր node-ը որպես head:
