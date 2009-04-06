#include "rootitem.h"

AtomEntry RootItem::sDummy;

const QString RootItem::kRootId = "Root-Id";

RootItem::RootItem(FeedsModel* model)
	: TreeItem(model, "/") {
	id_ = kRootId;
}
