/*
 * Copyright (c) 2021 Amir Czwink (amir130@hotmail.de)
 *
 * This file is part of ACBackup.
 *
 * ACBackup is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ACBackup is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ACBackup.  If not, see <http://www.gnu.org/licenses/>.
 */
//Local
#include "Nodes.hpp"
//Namespaces
using namespace StdXX;
using namespace StdXX::UI;

class RevisionsTree
{
public:
	//Constructor
	RevisionsTree()
	{
		this->root = new DirectoryTreeNode({u8"/"}, this->snapshotManager.NewestSnapshot());
	}

	//Properties
	inline TreeNode& Root()
	{
		return *this->root;
	}

private:
	//Members
	SnapshotManager snapshotManager;
	UniquePointer<TreeNode> root;
};

class RevisionsController : public TreeController
{
public:
	//Constructor
	RevisionsController(RevisionsTree& tree) : revisionsTree(tree)
	{
	}

	//Methods
	ControllerIndex GetChildIndex(uint32 row, uint32 column, const ControllerIndex &parent) const override
	{
		auto parentNode = this->ExtractNode(parent);
		return this->CreateIndex(row, column, parentNode->GetChildren()[row].operator->());
	}

	String GetColumnText(uint32 column) const override
	{
		return "File name";
	}

	uint32 GetNumberOfChildren(const ControllerIndex &parent) const override
	{
		if(parent == ControllerIndex())
			return this->revisionsTree.Root().GetChildren().GetNumberOfElements();
		return this->ExtractNode(parent)->GetChildren().GetNumberOfElements();
	}

	uint32 GetNumberOfColumns() const override
	{
		return 1;
	}

	ControllerIndex GetParentIndex(const ControllerIndex &index) const
	{
		NOT_IMPLEMENTED_ERROR; //TODO: implement me
	}

	String GetText(const ControllerIndex &index) const override
	{
		return this->ExtractNode(index)->GetName();
	}

private:
	//Members
	RevisionsTree& revisionsTree;

	//Inline
	inline TreeNode* ExtractNode(const ControllerIndex& index) const
	{
		if(index == ControllerIndex())
			return &this->revisionsTree.Root();
		return (TreeNode*)index.GetNode();
	}
};

int32 Main(const String& programName, const FixedArray<String>& args)
{
	if(args.GetNumberOfElements() != 1)
	{
		stdOut << u8"You need to pass the path to the backup directory" << endl;
		return EXIT_FAILURE;
	}

	auto& ic = InjectionContainer::Instance();
	ConfigManager configManager(args[0]);
	ic.ConfigManager(&configManager);

	RevisionsTree revisionsTree;

	EventHandling::StandardEventQueue eventQueue;

	MainAppWindow* wnd = new MainAppWindow(eventQueue);
	wnd->GetContentContainer()->SetLayout(new VerticalLayout);

	TreeView* treeView = new TreeView;
	treeView->SetController(new RevisionsController(revisionsTree));
	wnd->AddContentChild(treeView);

	PushButton* downloadButton = new PushButton;
	downloadButton->SetText(u8"Download");
	downloadButton->onActivatedHandler = [wnd, treeView]()
	{
		const auto& selection = treeView->SelectionController().GetSelectedIndexes();
		if(selection.IsEmpty())
			return;

		FileSystem::Path dirPath = wnd->SelectExistingDirectory(u8"Select download location");
		for(const auto& selectedItem : selection)
		{
			TreeNode* treeNode = (TreeNode*)selectedItem.GetNode();
			treeNode->DownloadTo(dirPath);
		}
	};
	wnd->AddContentChild(downloadButton);

	wnd->Show();

	eventQueue.ProcessEvents();

	return EXIT_SUCCESS;
}