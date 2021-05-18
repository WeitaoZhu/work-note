int src[2048][2048];
int dst[2048][2048];
	
void main(){
	int i,j;

	for (i=0;i<2048;i++)
	  for (j=0;j<2048;j++)
		dst[i][j]=src[i][j];
}