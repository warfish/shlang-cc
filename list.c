#if defined(TEST)

#include "list.h"
#include "test.h"

static void slist_test(void)
{
	slist_head slist;
	slist_init(&slist);
	CU_ASSERT_TRUE(slist_empty(&slist));

	slist_head node1 = SLIST_INIT;
	slist_head node2 = SLIST_INIT;
	slist_insert(&slist, &node2);
	slist_insert(&slist, &node1);

	CU_ASSERT_FALSE(slist_empty(&slist));
	CU_ASSERT_EQUAL(slist.next, &node1);
	CU_ASSERT_EQUAL(node1.next, &node2);
	CU_ASSERT_EQUAL(node2.next, NULL);

	slist_remove(&slist, &node1);
	CU_ASSERT_FALSE(slist_empty(&slist));
	CU_ASSERT_EQUAL(slist.next, &node2);
	CU_ASSERT_EQUAL(node2.next, NULL);

	slist_remove(&slist, &node2);
	CU_ASSERT_TRUE(slist_empty(&slist));
	CU_ASSERT_EQUAL(slist.next, NULL);
}
TEST_ADD(slist_test);

static void list_test(void)
{
	list_head list;
	list_init(&list);
	CU_ASSERT_TRUE(list_empty(&list));

	list_head node1, node2;
	list_insert(&list, &node2);
	list_insert(&list, &node1);

	CU_ASSERT_FALSE(list_empty(&list));
	CU_ASSERT_EQUAL(list.prev, NULL);
	CU_ASSERT_EQUAL(list.next, &node1);
	CU_ASSERT_EQUAL(node1.prev, &list);
	CU_ASSERT_EQUAL(node1.next, &node2);
	CU_ASSERT_EQUAL(node2.prev, &node1);
	CU_ASSERT_EQUAL(node2.next, NULL);

	list_remove(&node1);
	CU_ASSERT_FALSE(list_empty(&list));
	CU_ASSERT_EQUAL(list.prev, NULL);
	CU_ASSERT_EQUAL(list.next, &node2);
	CU_ASSERT_EQUAL(node2.prev, &list);
	CU_ASSERT_EQUAL(node2.next, NULL);

	list_remove(&node2);
	CU_ASSERT_TRUE(list_empty(&list));
	CU_ASSERT_EQUAL(list.next, NULL);
	CU_ASSERT_EQUAL(list.prev, NULL);
}
TEST_ADD(list_test);

#endif // TEST
