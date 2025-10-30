    if (head == nullptr) 
        return false;  // դատարկ list-ը ցիկլ չունի
//Երկու ցուցիչ դնում ենք սկզբում
    Node* slow = head;
    Node* fast = head;

//ապահովում է, որ երբ փորձում ենք fast->next->next-ը հասնել, չհասցնենք nullptr-ին(չլինի crash)
    while (fast != nullptr && fast->next != nullptr) {
        slow = slow->next;   // 1 քայլ
        fast = fast->next->next; // 2 քայլ

        if (slow == fast) {
            // Եթե հանդիպեցին՝ ցիկլ կա
            return true;
        }
    }
    // Եթե fast հասավ NULL — ցիկլ չկա
    return false;