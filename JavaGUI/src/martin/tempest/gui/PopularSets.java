package martin.tempest.gui;

import java.util.HashSet;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.List;
import java.util.Collections; 
import java.util.Comparator;
public class PopularSets{
	private final String name;
	private Boolean dirty;
	private HashSet<PopularVideoMode> set;
	public class PopularVideoMode {
		protected int popularity;
		private VideoMode v;
		public PopularVideoMode(VideoMode videoModeObjReference, int popularity){
			this.v=videoModeObjReference; 
			this.popularity = popularity; 
		}
		public int getPopularity(){
			return this.popularity;
		}
		public void setPopularity(int popularity){
			this.popularity=popularity; 
		}
		//TOCHECK
		public String getName(){
			return v.name;
		}
		/**
		 * 
		 * @param n number of scales to apply, n=1 will double, n=-1 will half, 0 representing identity
		 * @return
		 */
		public double getFrequency(Integer n){
			int tmpN=0; 
			if(n!=null){
				tmpN=n.intValue(); 
			}
			double freq= (this.v.height*this.v.width*this.v.refreshrate); 
			if(freq>=0){
				return freq*(n+1);
			}
			return freq*1/((-n)+1);
			
		}
	}
	//Ensures that its not adding a repeatedVideoMode
	public void addMode(PopularVideoMode t){
		
		//check.v=; 
		if(!set.stream().anyMatch(dto -> dto.v==t.v)){
			set.add(t);
			this.dirty=Boolean.TRUE; 
		}
		/*if(){
			set.add(t); 
		
		}*/
			
		
	}
	//Ensures repeated values are removed(this is just redudancy)
	public void removeMode(VideoMode t){
		if(set.removeIf(z->z.v.equals(t))) {
			this.dirty=Boolean.TRUE; 
		}
	}
	public List<PopularVideoMode> sort(){
		//set.values();
		List<PopularVideoMode> VideoModesByPopularity = new ArrayList<>(set);
		if(!dirty){
			return VideoModesByPopularity;
		}
		dirty=Boolean.FALSE; 
		Collections.sort(VideoModesByPopularity, Comparator.comparing(PopularVideoMode::getPopularity));
		return VideoModesByPopularity; 
	}
	public Iterator<PopularVideoMode> getIterator(){
		List<PopularVideoMode> sortedList = this.sort();
		return sortedList.iterator();
	}
	public String getName(){
		return this.name; 
	}
	public Boolean isDirty(){
		Boolean t=this.dirty;
		return t; 
	}
	public PopularSets(String name){
		this.dirty=Boolean.FALSE;
		this.name=name;
		set= new HashSet<PopularVideoMode>();//new HashMap<String, PopularVideoMode>(); 
	}
	
	
	
}
