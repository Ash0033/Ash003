 int max=INT_MIN //<climits> գրադարանի const;  
 for (int i = 0; i <= 3; i++) {
    for (int j = 0; j <= 3; j++) {
        int gumar = arr[i][j] + arr[i][j+1] + arr[i][j+2] + arr[i+1][j+1] + arr[i+2][j] + arr[i+2][j+1] + arr[i+2][j+2];
       //3հատ վերևից,1հատ մեջտեղից,3հատ էլ ներքևից
       if(gumar>max)
                   max=gumar;
                   //կամ կգրենք max = std::max(max, gumar); //ֆունկցիայով, դրա համար կարիք չի լինի վերևի գրածները
    }
}
return max; 