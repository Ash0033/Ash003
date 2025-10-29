SinglyLinkedListNode* newNode = new SinglyLinkedListNode(data);// Ստեղծում ենք նոր node
    // Եթե պետք է ավելացնենք հենց սկզբում
    if (position == 0) //եթե 0ա ուրեմն նոր տարրը կդառնա առաջինը
    {
        newNode->next = llist; //նոր node-ի next-ը նախկին head-ին
        return newNode; 
    }

    // Գնում ենք մինչև այն հանգույցը, որի հետո պետք է ավելացնենք
    SinglyLinkedListNode* current = llist; //current ցուցիչը դառնում է հենց head-ը,ցույց է տալիս առաջին node-ը
    for (int i = 0; i < position - 1; i++) {
        current = current->next;
    }

    // Կապում ենք նոր node-ը
    newNode->next = current->next;
    current->next = newNode;

    // Վերադարձնում ենք ամբողջ head-ը
    return llist;

