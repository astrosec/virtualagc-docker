058931,000668:                                                                                                  #  TFF CONSTANTS
058932,000669: 
058933,000670: 32,3770                                           BANK     32                                    
058934,000671: 
058935,000672: 27,2000                                           SETLOC   TOF-FF1                               
058936,000673: 27,2000                                           BANK                                           
058937,000674: 
058938,000675:                                                                                                  #                                                NOTE _  NOTE _ ADJUSTED MUE FOR NEAR EARTH TRAJ.
058939,000676:                                                                                                  # MUE            =       3.990815471 E10         M CUBE/CS SQ
058940,000677:                                                                                                  # RTMUE          =       1.997702549 E5 B-18*    MODIFIED EARTH MU
058941,000678: 
058942,000679: 27,3353           24775 30424  1/RTMU             2DEC*    .5005750271 E-5        B17*            #  MODIFIED EARTH MU
058943,000680: 
058944,000681:                                                                                                  #                                                NOTE _  NOTE _ ADJUSTED MUE FOR NEAR EARTH TRAJ.
058945,000682:                                                                                                  # MUM            =       4.902778 E8             M CUBE/CS SQ
058946,000683:                                                                                                  # RTMUM          2DEC*   2.21422176 E4 B-18*
058947,000684: 27,3355           06220 37553  PI/16              2DEC     3.141592653 B-4                        
058948,000685: 27,3357           37777 37700  LIM(-22)           2OCT     3777737700                            #  1.0 -B(-22)
058949,000686: 27,3361           00000 00100  DP(-22)            2OCT     0000000100                            #  B(-22)
058950,000687: 27,3363           04000 00000  DP2(-3)            2DEC     1          B-3                        
058951,000688: 27,3365           02000 00000  DP2(-4)            2DEC     1          B-4                        #  1/16
058952,000689: 
058953,000690:                                                                                                  #  RPAD1         2DEC    6373338 B-29            M (-29) = 20909901.57 FT
058954,000691: 27,3367  22,3151               RPAD1              =        RPAD                                  
058955,000692: 
058956,000693: 27,3367           00305 11205  R300K              2DEC     6464778    B-29                       #  (-29) M
058957,000694: 27,3371           37777 37777  NEARONE            2DEC     .999999999                            
058958,000695: 27,3373  26,3334               TFFZEROS           EQUALS   HI6ZEROS                              
058959,000696: 27,3373  26,3324               TFF1/4             EQUALS   HIDP1/4                               
