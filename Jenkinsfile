pipeline 
{
	agent any

	stages
	{
		stage('Build') 
		{
			steps
			{
				
				sh 'cd ./bin/posix'
				sh 'cmake -DOpENer_PLATFORM:STRING="POSIX" -DOpENer_TESTS:BOOL="ON" ../../source' 
				sh 'make all -j4'
			}
		}
		stage('Test') 
		{
			steps
			{
				sh 'ctest -j4'
			}
		}
		stage('Install')
		{
			steps
			{
				sh 'make install -j4'
			}
		}
	}	
}
