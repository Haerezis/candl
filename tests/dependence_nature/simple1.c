#pragma scop
for(i=1 ; i<N1 ; i++)
    for(j=1 ; j<N2 ; j++)
        for(k=1 ; k<N3 ; k++)
            A[i][j][k] = A[i-1][j][k-1];
#pragma endscop
