    DoublyLinkedListNode* current = llist;//ցույց է տալիս այն node-ը, որի վրա հիմա ենք
    DoublyLinkedListNode* temp = nullptr;//ժամանակավոր փոփոխական, որը պահելու է մեկ pointer, երբ մենք դրանք փոխում ենք

    // Գնալ բոլոր node-երով
    //Քանի դեռ current գոյություն ունի (չենք հասել ցանկի վերջը), պետք է փոխենք տվյալ node-ի կապերը։
    while (current != nullptr) {
        // Փոխել next և prev
        temp = current->prev; //պահում ենք prev արժեքը
        current->prev = current->next;
        current->next = temp;

        // Գնալ հաջորդ node (նախկին next-ը հիմա prev է)
        current = current->prev;
    }

    //head-ը փոխվել է վերջին node-ի վրա
    if (temp != nullptr)
        llist = temp->prev;

    return llist;
