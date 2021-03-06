// Copyright 2018 Krystian Stasiowski

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <set>
#include <chrono>
#include <string>
#include <windows.h>
#include <experimental/filesystem>

const std::vector<char> MOVES_X = { 1, -1, 0, 0 };
const std::vector<char> MOVES_Y = { 0, 0, 1, -1 };

struct Vector2
{
  int x;
  int y;

  Vector2() : x(0), y(0) { };
  Vector2(int _x, int _y) : x(_x), y(_y) { };

  bool operator==(const Vector2& other)
  {
    return x == other.x && y == other.y;
  }
};

struct Node
{
  Vector2 position;
  unsigned int moves;
  int parent;

  Node(int _x, int _y, int _moves)
  {
    position.x = _x;
    position.y = _y;
    moves = _moves;
  };

  Node(int _x, int _y, int _moves, int _parent)
  {
    position.x = _x;
    position.y = _y;
    moves = _moves;
    parent = _parent;
  };

  Node() = default;

  bool operator==(const Node& other)
  {
    return position == other.position;
  }
};

struct Maze
{
  std::vector<std::vector<char>> map;
  Vector2 entry;
  Vector2 exit;
  int sizex;
  int sizey;
};

bool Valid(const Vector2& pos, int szx, int szy)
{
  return pos.x >= 0 && pos.x < szx && pos.y >= 0 && pos.y < szy;
}

std::vector<Vector2> ReconstructPath(std::vector<Node>& nodes, Node node)
{
  std::vector<Vector2> path;
  while (node.parent != -1)
  {
    path.push_back(node.position);
    node = nodes[node.parent];
  }
  return path;
}

std::vector<Vector2> Path(Maze& maze)
{
  int nodes = 0;
  std::queue<Node> moves;
  std::vector<Node> used;
  used.resize(maze.sizex * maze.sizey);
  moves.push(Node(maze.entry.x, maze.entry.y, 1, -1));
  while (!moves.empty())
  {
    Node node(moves.front());
    used.push_back(node);
    moves.pop();
    if (node.position == maze.exit)
    {
      return ReconstructPath(used, node);
    }
    if (maze.map[node.position.x][node.position.y] != 219)
    {
      for (int i = 0; i < 4; ++i)
      {
        Vector2 newpos(node.position.x + MOVES_X[i], node.position.y + MOVES_Y[i]);
        Node newnode = Node(newpos.x, newpos.y, node.moves + 1, used.size() - 1);
        if (Valid(newpos, maze.sizex, maze.sizey) && maze.map[newpos.x][newpos.y] == 32)
        {
          maze.map[newnode.position.x][newnode.position.y] = 176;
          ++nodes;
          moves.push(newnode);
        }
      }
    }
  }
  return std::vector<Vector2>();
}

int main()
{
  cv::Mat image;
  Maze maze;
  std::cout << "Enter name of maze file and extention: ";
  std::string selection;
  std::cin >> selection;
  std::cin.clear();
  image = cv::imread(std::experimental::filesystem::current_path().string() + "\\mazes\\" + selection, CV_LOAD_IMAGE_COLOR);
  if (!image.data)
  {
    std::cout << "Could not open or find the image" << std::endl;
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)());
    return -1;
  }
  std::vector<std::vector<char>> map(image.cols);
  for (std::vector<char>& vec : map)
  {
    vec.resize(image.rows);
  }
  std::cout << "Reading image..." << std::endl;
  maze.sizex = image.cols;
  maze.sizey = image.rows;
  for (int x = 0; x < image.cols; ++x)
  {
    for (int y = 0; y < image.rows; ++y)
    {
      cv::Vec3b color = image.at<cv::Vec3b>(cv::Point(y, x));
      if (color[2] == 0 && color[1] == 255 && color[0] == 0)
      {
        std::cout << "Found entry at " << x << ", " << y << std::endl;
        maze.entry = Vector2(x, y);
      }
      if (color[2] == 255 && color[1] == 0 && color[0] == 0)
      {
        std::cout << "Found exit at " << x << ", " << y << std::endl;
        maze.exit = Vector2(x, y);
      }
      map[x][y] = color[1] == 0 && color[2] == 0 ? 219 : 32;
    }
  }
  maze.map = map;
  std::chrono::high_resolution_clock::time_point begin = std::chrono::high_resolution_clock::now();
  std::vector<Vector2> path = Path(maze);
  std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
  long long duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
  std::cout << "Path length " << path.size() << " found in " << duration << " microseconds" << std::endl;
  std::reverse(path.begin(), path.end());
  std::cout << "Visualize path? ";
  bool visualize;
  std::cin >> visualize;
  std::cin.clear();
  if (visualize)
  {
    cv::namedWindow("Maze", cv::WINDOW_AUTOSIZE);
    for (Vector2 v : path)
    {
      cv::Vec3b color = image.at<cv::Vec3b>(cv::Point(v.y, v.x));
      color[0] = 255;
      color[1] = 0;
      color[2] = 0;
      image.at<cv::Vec3b>(cv::Point(v.y, v.x)) = color;
      cv::Mat render;
      cv::resize(image, render, cv::Size(1000, 1000), cv::INTER_LINEAR);
      imshow("Maze", render);
      cv::waitKey(1);
    }
  }
  cv::waitKey();
  return 0;
}