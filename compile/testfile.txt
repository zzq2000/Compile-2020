void main()
{
    int N[2][4] = {{1,2,3,4},{5,6,7,8}};
    int i, j;
    if (N[0][0] == 1)
    {
        printf("18231210");
    }

    for (i = 0; i < 2; i=i+1)
    {
        for (j = 0; j < 4; j=j+1)
        {
            switch (N[i][j])
            {
            case 1:
                printf(1);
            case 2:
                printf(2);
            case 3:
                printf(3);
            case 4:
                printf(4);
            case 5:
                printf(5);
            case 6:
                printf(6);
            case 7:
                printf(7);
            default:
                printf(8);
            }
        }
    }

    switch (N[0][0])
    {
        case 1:
            N[0][0] = 0;
        default:
            N[0][0] = 1;
	}

    if (N[0][0] != 2)
    {
        printf("finish");
    }
}