// // blackout back/up
// 18
// 17,28
// 16,27,38
// 15,26,37,48
// 14,25,36,47,58
// 13,24,35,46,57,68
// 12,23,34,45,56,67,78
// 11,22,33,44,55,66,77,88
//    21,32,43,54,65,76,87
//       31,42,53,64,75,86
//          41,52,63,74,85
//             51,62,73,84
//                61,72,83
//                   71,82
//                      81
// 
// // arrow back/up
// 18
// 17,28
//    27
//    26,37
//       36,
//       35,46
//          45
//          44,55
//             54
//             53,64
//                63
//             51,62,73,84
//                61,72,83
//                   71,82
//                      81
// 
// // blackout forward/down
// 81
// 82,71
// 83,72,61
// 84,73,62,51
// 85,74,63,52,41
// 86,75,64,53,42,31
// 87,76,65,54,43,32,21
// 88,77,66,55,44,33,22,11
//    78,67,56,45,34,23,12
//       68,57,46,35,24,13
//          58,47,36,25,14
//             48,37,26,15
//                38,27,16
//                   28,17
//                      18
// 
// // arrow forward/down
//                      81
//                   71,82
//                   72
//                62,73
//                63
//             53,64
//             54
//          44,55
//          45
//       35,46
//       36
// 15,26,37,48
// 16,27,38
// 17,28
// 18
// 
// // startup whites
// 11
// 12,21
//       31
// 14,23,   41
//    24,33,   51
// 16,   34,43,   61
//    26,      53
// 18,   36,45,   63,72
//    28,   46,55,   73,82
//       38,   56,65,   83
//          48,      75
//             58,67,   85
//                68,77
//                   78,87
// 
// // test // init
// unsigned long a_time = 125;
// unsigned long multi = 0;
// // test // function
// if(currentMillis - previousMillis >= (a_time * multi++)){
//   for (int pad = 0; pad<sizeofarray; ++pad){
//   send(pad,r,g,b)
//   }
// }
// if(currentMillis - previousMillis >= (a_time * multi++)){ }
// multi = 0;
// 
// 
// // template
// 81,   82,   83,   84,   85,   86,   87,   88
// 
// 71,   72,   73,   74,   75,   76,   77,   78
// 
// 61,   62,   63,   64,   65,   66,   67,   68
// 
// 51,   52,   53,   54,   55,   56,   57,   58
// 
// 41,   42,   43,   44,   45,   46,   47,   48
// 
// 31,   32,   33,   34,   35,   36,   37,   38
// 
// 21,   22,   23,   24,   25,   26,   27,   28
// 
// 11,   12,   13,   14,   15,   16,   17,   18
// 
// Cue:
// 81,82,83,84,85,86,87,88, 73,74,78,68,58,48, 32,35,36,37,38 22,23,24,25,26,27,28, 11,12,13,14,15,16,17,18 - 0,0,0
// 71 - 96,32,0
// 61,51,41,31 - 64,32,0
// 21 - 48,16,0
// 72,76,63,65,67,52,54,56,43,45,47,24 - 46,46,46
// 75,77,62,64,66,53,55,57,42,44,46,33,35,37,24 - 96,96,96


