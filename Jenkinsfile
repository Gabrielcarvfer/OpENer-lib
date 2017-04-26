pipeline 
{
	agent any

	stages
	{
		stage('Build') 
		{
			steps
			{
				
				sh 'cd ./bin/posix && cmake -DOpENer_PLATFORM:STRING="POSIX" ../../source' 
				sh 'make all'
			}
		}
		stage('Test') 
		{
			steps
			{
				sh 'ctest'
			}
		}
		stage('Install')
		{
			steps
			{
				sh 'make install'
			}
		}
	}	
}