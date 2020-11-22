#ifndef __W_STL_TREE__
#define __W_STL_TREE__

namespace wang
{

template <class T>
inline void swap(T& a, T& b) {
  T tmp = a;
  a = b;
  b = tmp;
}


struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};


typedef bool _Rb_tree_node_color_type;
const _Rb_tree_node_color_type _S_rb_tree_red = true;
const _Rb_tree_node_color_type _S_rb_tree_black = false;

struct _Rb_tree_node_base
{
	typedef _Rb_tree_node_base* _Base_ptr;
	typedef _Rb_tree_node_color_type _Color_type;

	_Color_type m_color;
	_Base_ptr	m_parent;
	_Base_ptr   m_left;
	_Base_ptr	m_right;

	static _Base_ptr minimum(_Base_ptr x)
	{
		while (x->m_left != NULL) x = x->m_left;
		return x;
	}

	static _Base_ptr maximum(_Base_ptr x)
	{
		while (x->m_right != NULL) x = x->m_right;
		return x;
	}
	
};

template <class Value>
struct _Rb_tree_node : public _Rb_tree_node_base
{
	typedef _Rb_tree_node<Value> _Link_type;
	typedef Value _Value_type;
};


struct _Rb_tree_base_iterator
{

	typedef _Rb_tree_node_base::_Base_ptr _Base_ptr;
	typedef bidirectional_iterator_tag iterator_category;

	_Base_ptr m_node;

	void increment()
	{
		if (m_node->m_right != NULL)
		{
			m_node = m_node->m_right;
			while (m_node->m_left != NULL)
			{
				m_node = m_node->m_left;
			}	
		}
		else
		{
			_Base_ptr y = m_node->m_parent;
			while (y->m_right == m_node)
			{
				m_node = y;
				y = y->m_parent;
			}
			
			//只有一个根节点时，防止 m_node 变成 header 节点
			if (m_node->m_right != y) 
			{
				m_node = y;
			}
		}
	}

	void decrement()
	{
		if (m_node->m_color == _S_rb_tree_red && 
			m_node->m_parent->m_parent == m_node)
		{
			//节点为红，且节点的祖父节点为该节点，表示该节点
			m_node = m_node->m_right;
		}
		else if (m_node->m_left != NULL)
		{
			_Base_ptr y = m_node->m_left;
			while (y->m_right != NULL)
			{
				y = y->m_right;
			}
			m_node = y;
		}
		else
		{
			_Base_ptr y = m_node->m_parent;
			while (y->m_left == m_node)
			{
				m_node = y;
				y = y->m_parent;
			}
			m_node = y;
		}
	}

};

template <class Value, class Ref, class Ptr>
struct _Rb_tree_iterator : public _Rb_tree_base_iterator
{
	typedef Value 	value_type;
	typedef Ref		reference;
	typedef Ptr		pointer;
	typedef _Rb_tree_iterator<Value, Value&, Value*>	iterator;
	typedef _Rb_tree_iterator<Value, const Value&, const Value*> const_iterator;
	typedef _Rb_tree_iterator<Value, Ref, Ptr> _Self;
	typedef _Rb_tree_node<Value> * _Link_type;

	_Rb_tree_iterator() {}
	_Rb_tree_iterator(_Link_type x) {m_node = x;}
	_Rb_tree_iterator(const iterator& iter) {m_node = iter.m_node; }

	reference operator*() const { return _Link_type(m_node)->m_value; }

	_Self &operator++() { increment(); return *this; }
	_Self operator++(int) 
	{
		//++i 和 i++ 比较，i++ 会多一次迭代器构造过程
		//目前一个迭代器实例中包含一个元素指针
		_Self tmp = *this;
		increment();
		return tmp;
	}

	_Self &operator--() { decrement() ; return *this; }
	_Self operator--(int) 
	{
		_Self tmp = *this;
		decrement();
		return tmp;
	}	
};

inline bool operator==(const _Rb_tree_base_iterator& x, 
							const _Rb_tree_base_iterator& y)
{
	return x.m_node == y.m_node;
}

inline bool operator!=(const _Rb_tree_base_iterator& x, 
							const _Rb_tree_base_iterator &y)
{
	return x.m_node != y.m_node;
}

//左旋，即是基点往下移一层，其右孩子往上移一层，右孩子的节点变成该基点
// 基点的右边挂上右孩子的左子树
void _Rb_tree_rotate_left(_Rb_tree_node_base* x, _Rb_tree_node_base* &root)
{
	_Rb_tree_node_base* y = x->m_right;
	//x 右孩子的左子树挂到x的右边
	x->m_right = y->m_left;
	if (y->m_left != NULL)
	{
		y->m_left->m_parent = x;
	}
	
	// x 的 右子树 挂到 x父节点的右边
	y->m_parent = x->m_parent;

	if (x == root)
	{
		root = y;
	}
	else if (x == x->m_parent->m_left)
	{
		x->m_parent->m_left = y;
	}
	else
	{
		x->m_parent->m_right = y;
	}

	y->m_left = x;
	x->m_parent = y;
}

//原理同左旋
void _Rb_tree_rotate_right(_Rb_tree_node_base* x, _Rb_tree_node_base* &root)
{
	_Rb_tree_node_base *y = x->m_left;
	x->m_left = y->m_right;

	if (y->m_right != NULL)
	{
		y->m_right->m_parent = x;
	}

	y->m_parent = x->m_parent;

	if (x == root)
	{
		root = x;
	}
	else if (x == x->m_parent->m_left)
	{
		x->m_parent->m_left = y;
	}
	else 
	{
		x->m_parent->m_right = y;
	}

	y->m_right = x;
	x->m_parent = y;
}

//父节点为红的情况才需要平衡
inline void _Rb_tree_rebalance(_Rb_tree_node_base *x, _Rb_tree_node_base *& root)
{
	x->m_color = _S_rb_tree_red;
	
	while (x != root && x->m_parent->m_color == _S_rb_tree_red)
	{
		// x的父节点是其祖父节点的左孩子
		if (x->m_parent == x->m_parent->m_parent->m_left)
		{
			_Rb_tree_node_base *y = x->m_parent->m_parent->m_right;
			//父节点的兄弟节点和父节点一样为红色，统一变黑，x变成x的祖父节点
			if (y && y->m_color == _S_rb_tree_red)
			{
				x->m_parent->m_color = _S_rb_tree_black;
				y->m_color = _S_rb_tree_black;
				y->m_parent->m_color = _S_rb_tree_red;
				x = y->m_parent;
			}
			else 
			{
				//左右 --> 左左 : 让x变成x的父节点，以x为基点，左旋
				if (x = x->m_parent->m_right)
				{
					x = x->m_parent;
					_Rb_tree_rotate_left(x, root);
				}
				//x 父节点变黑，祖父节点变红，以祖父节点为基点，右旋
				x->m_parent->m_color = _S_rb_tree_black;
				x->m_parent->m_parent->m_color = _S_rb_tree_red;
				_Rb_tree_rotate_right(x->m_parent->m_parent, root);
			}
		}
		else //对称情况
		{
			_Rb_tree_node_base *y = x->m_parent->m_parent->m_right;
			//父节点的兄弟节点和父节点一样为红色，统一变黑，x变成x的祖父节点
			if (y && y->m_color == _S_rb_tree_red)
			{
				x->m_parent->m_color = _S_rb_tree_black;
				y->m_color = _S_rb_tree_black;
				y->m_parent->m_color = _S_rb_tree_red;
				x = y->m_parent;
			}
			else
			{
				//右左 --> 右右 : 让x变成x的父节点，以x为基点，右旋
				if (x = x->m_parent->m_left)
				{
					x = x->m_parent;
					_Rb_tree_rotate_right(x, root);
				}
				//x 父节点变黑，祖父节点变红，以祖父节点为基点，左旋
				x->m_parent->m_color = _S_rb_tree_black;
				x->m_parent->m_parent->m_color = _S_rb_tree_red;
				_Rb_tree_rotate_left(x->m_parent->m_parent, root);
			}			
		}
	}

	// 保证根节点为黑色
	root->m_color = _S_rb_tree_black;
}


inline _Rb_tree_node_base *_Rb_tree_reblance_for_erase(_Rb_tree_node_base *z,
																	_Rb_tree_node_base *& root,
																	_Rb_tree_node_base *& leftmost,
																	_Rb_tree_node_base *& rightmost)
{
	// y为最终要删除节点
	_Rb_tree_node_base *y = z;
	_Rb_tree_node_base *x = NULL; //x 为要删除节点(真)的子节点
	_Rb_tree_node_base *x_parent = NULL;

	// z最多有一个非空孩子，取其必定为空的孩子之外的孩子，y == z，x可能为空
	if (y->m_left == NULL)
	{
		x = y->m_right;
	}
	else if (y->m_right == NULL) //同上
	{
		x = y->m_left;
	}
	else
	{
		// 中序遍历寻找后继节点
		
		// z 有两个非空孩子，设置y为z的后继节点，x可能为空
		// 删除节点的后继节点一定是删除节点右子树的最左侧节点
		y = y->m_right;
		while (y->m_left != NULL)
		{
			y = y->m_left;
		}
		//此时 y左子树为空，右子树未知，如果非空，可能最终让其挂在y父节点的左边
		x = y->m_right;
	}

	//不相等，即z拥有左右孩子的情况
	if (y != z) //重新链接y 取代z (y为z的后继节点)
	{
		// z的左子树 先挂到后继节点y上的左边 (y左子树为空)
		z->m_left->m_parent = y;
		y->m_left = z->m_left;

		// y 不为 z的右孩子，即y和z中间隔着n(n >= 1)层
		if (y != z->m_right)
		{
			x_parent = y->m_parent;
			if (x) 
			{
				// x为y的孩子，y要替代z的位置，那么x则得挂到y的父节点上
				x->m_parent = y->m_parent;
			}
			// 由前面判断可知，y必定为其父节点的左孩子
			y->m_parent->m_left = x;

			// z的 右子树 挂到y的右边
			// 如果直接 y == z->m_right 的情况，无需做如下处理 --
			y->m_right = z->m_right;
			z->m_right->m_parent = y;
		}
		else
		{
			//y 为 z的右孩子，y替代z后，其孩子节点继续挂在它上面
			x_parent = y;
		}
		
		// 前面跟z的孩子建立连接，现在跟z的父节点建立连接
		if (root == z)
		{
			root = y;
		}
		else if (z->m_parent->m_left = z)
		{
			z->m_parent->m_left = y;
		}
		else
		{
			z->m_parent->m_right = y;
		}
		
		y->m_parent = z->m_parent;

		// 同样 y继承 z的颜色
		swap(y->m_color, z->m_color);

		// 将z赋值为y，实际要开始删除z了
		y = z;
	}
	else // y == z
	{
		// 去除y节点，将其子树挂在y的父节点上
		x_parent = y->m_parent;
		if (x)
		{
			x->m_parent = y->m_parent;
		}
		if (root == z)
		{
			root = x;
		}
		else if (z->m_parent->m_left == z)
		{
			z->m_parent->m_left = x;		
		}
		else
		{
			z->m_parent->m_right = x;
		}

		// 只有删除的节点没有两个孩子的情况，才有可能需要调整最大节点和最小节点
		if (leftmost == z)
		{
			// z为最小值，则z只有右子树，如果右孩子还为空，那么最小节点为其父节点
			if (z->m_right == NULL)
			{
				leftmost = z->m_parent;
			}
			else // 在其右子树上找最小值
			{
				leftmost = _Rb_tree_node_base::minimum(x);
			}
		}

		// 意义同上
		if (rightmost == z)
		{
			if (z->m_left == NULL)
			{
				rightmost = z->m_parent;
			}
			else 
			{
				rightmost = _Rb_tree_node_base::maximum(x);
			}
		}
	}

	//删除节点为红色时，树层级不变，不需要做额外处理
	if (y->m_color != _S_rb_tree_red)
	{
		//往上递归 处理
		while (x != root && (x == NULL || x->m_color == _S_rb_tree_black))
		{
			if (x == x_parent->m_left)
			{
				_Rb_tree_node_base* w = x_parent->m_right;
				
				if (w->m_color == _S_rb_tree_red)
				{
					w->m_color = _S_rb_tree_black;
					x_parent->m_color = _S_rb_tree_red;
					_Rb_tree_rotate_left(x_parent, root);
					x = x_parent->m_right;
				}

				if ((w->m_left == 0 || w->m_left->m_color == _S_rb_tree_black) &&
					(w->m_right == 0 || w->m_right->m_color == _S_rb_tree_black))
				{
					w->m_color = _S_rb_tree_red;
					x = x_parent;
					x_parent = x_parent->m_parent;
				}
				else
				{
				
				}

			}
		}
	}

}

}






#endif

