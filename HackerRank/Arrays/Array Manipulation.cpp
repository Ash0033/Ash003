vector<long> diff(n + 2, 0); //n+2 out of range չլինելու համար 
   for (auto &q : queries) { 
//auto գրելիս կոմպիլյատորը նայում է աջ կողմի արժեքին ու որոշում՝ ինչ տիպ պետք է լինի
    //(vector<int> &q : queries)
    //Եթե չգրեինք & յուր. քայլում queries-ի ներսի փոքր vector-ը կպատճենվեր
    int a = q[0]; //սկսվող ինդեքս
    int b = q[1]; //վերջնական ինդեքս
    int k = q[2]; //ավելացվող արժեքը
        diff[a] += k; 
        diff[b + 1] -= k; } 
    
    long max_val = 0; 
    long current = 0; //ընթացիկ prefix sum-ը  
        for (int i = 1; i <= n; i++) { 
        current += diff[i]; 
        if (current > max_val) max_val = current; 
        } 
        return max_val;