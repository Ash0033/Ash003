 SinglyLinkedListNode* a = head1;//a ցույց է տալիս առաջին ցուցակի սկզբին
    SinglyLinkedListNode* b = head2;//b ցույց է տալիս երկրորդ ցուցակի սկզբին

    // Այս ցիկլը կավարտվի, երբ a == b
    while (a != b) {
        // Եթե a հասել է վերջ, ուղարկում ենք նրան head2-ի վրա
        a = (a == nullptr) ? head2 : a->next;

        // Եթե b հասել է վերջ, ուղարկում ենք նրան head1-ի վրա
        b = (b == nullptr) ? head1 : b->next;
    }

    // Երբ նրանք կհանդիպեն, դա merge node-ն է
    return a->data;
