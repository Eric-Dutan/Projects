package flappyBird;

import java.awt.Graphics;

import javax.swing.JPanel;

public class Renderer extends JPanel{

	private static final long serialVersionUID = 8095379131479840912L;
	
	@Override
	protected void paintComponent(Graphics g) {
		// TODO Auto-generated method stub
		super.paintComponent(g);
		
		FlappyBird.flappyBird.repaint(g);
	}

}
