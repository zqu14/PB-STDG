//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package swarmstack.beaconing.processor;

simple Processor 
{
    parameters:
        double  minimumSafetyDistance @unit(m)   =  default(0.5m);
        double  maxNeighborRange @unit(m)        =  default(100m);
        double  clTimeThreshold @unit(s)         =  default(1.5s);
        double  overtakeSpeed                    =  default(1);
        double  E1 @unit(m)    =  default(30m);
        double  E2 @unit(m)    =  default(16m);
        double  E3 @unit(m)    =  default(30m);
        double K1 = default(10);
        double K2 = default(1);
        double K3 = default(300);
        double KI = default(0.3);
        string fisrtPart= default("0");
        string endPart= default("0");
        int currentI = default(0);
        int currentJ= default(0); 
        double enableOvertake = default(5);
        volatile int randomTheta                 = default(intuniform(0, 359));
        volatile double random01                = default(uniform(0,1));
        
        bool isback = default(true);
        
        @signal[overtakeCount](type=bool);
		@statistic[overtakeCount](record=count);
		
		@signal[perferredSpeedValue](type=double);
		@statistic[perferredSpeedValue](record=mean);
		
		@signal[switchCost](type=double);
		@statistic[switchCost](record=mean);
		
		@signal[speedCost](type=double);
		@statistic[speedCost](record=mean);
		
		@signal[positionCost](type=double);
		@statistic[positionCost](record=mean);
		
		@signal[totalCost](type=double);
		@statistic[totalCost](record=mean);
		
	gates:
	    
	    input  beaconsIn;
	    input  mIn;
	    output toM;
	    output toRB;
	    
	    
}