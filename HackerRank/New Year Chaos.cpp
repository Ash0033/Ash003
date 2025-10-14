 int bribes = 0;
    int n = q.size();

    for (int i = 0; i < n; i++) {
        // Եթե մարդը իր սկզբնական տեղից ավելի քան 2 դիրքով առաջ է անցել
        if (q[i] - (i + 1) > 2) //q[i]-ն սկզբնական դիրք, (i+1)-ն վերջնական
         {
            cout << "Too chaotic" << endl;
            return;     }}