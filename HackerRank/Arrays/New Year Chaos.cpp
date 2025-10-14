 int bribes = 0;
    int n = q.size();
       for (int i = 0; i < n; i++) {
        // Եթե մարդը իր սկզբնական տեղից ավելի քան 2 դիրքով առաջ է անցել
        if (q[i] - (i + 1) > 2) //q[i]-ն սկզբնական դիրք, (i+1)-ն վերջնական
         {
            cout << "Too chaotic" << endl;
            return;     }
           // Ստուգում ենք՝ քանի հոգու է նա bribed արել
        for (int j = max(0, q[i] - 2); j < i; j++) {
            if (q[j] > q[i]) bribes++;
        }
    }
    cout << bribes << endl;

 