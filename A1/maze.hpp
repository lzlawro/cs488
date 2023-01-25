// Termm--Fall 2020

#pragma once

class Maze
{
public:
	Maze( size_t dim );
	~Maze();

	void reset();

	size_t getDim() const;

	int getValue( int x, int y ) const;

	int getAvatarR();
	int getAvatarC();

	void setValue( int x, int y, int h );

	void setAvatarValue(int x, int y);

	void digMaze();
	void printMaze(); // for debugging
	void printAvatarValue();

private:
	size_t m_dim;

	int m_avatar_r;
	int m_avatar_c;

	unsigned int m_wall_count;

	int *m_values;
	void recDigMaze(int r, int c);
	int numNeighbors(int r, int c);
};
