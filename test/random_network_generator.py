import json
import random
import string
from datetime import datetime
from typing import List, Dict, Set, Tuple, Optional
import argparse
import os
import math


class EnhancedNetworkGenerator:
    def __init__(self, seed=None):
        """初始化随机数生成器"""
        if seed:
            random.seed(seed)
        else:
            random.seed(datetime.now().timestamp())
        
        # 丰富的姓名池
        self.first_names = [
            "Alex", "Ben", "Chris", "David", "Ethan", "Frank", "George", "Henry",
            "Ian", "Jack", "Kevin", "Leo", "Mike", "Noah", "Oscar", "Peter",
            "Quinn", "Ryan", "Steve", "Tom", "Alice", "Bella", "Cindy", "Diana",
            "Emma", "Fiona", "Grace", "Hannah", "Iris", "Julia", "Kate", "Lily",
            "Mona", "Nina", "Olive", "Paula", "Queenie", "Rose", "Sara", "Tina",
            "Uma", "Vera", "Wendy", "Xena", "Yvonne", "Zoe", "Aaron", "Bruce",
            "Clark", "Derek", "Evan", "Felix", "Gavin", "Hugo", "Ivan", "Jake",
            "Kyle", "Liam", "Mason", "Nathan", "Oliver", "Paul", "Quentin", "Reed",
            "Simon", "Toby", "Ulysses", "Victor", "Walter", "Xavier", "Yale", "Zane"
        ]
        
        self.last_names = [
            "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller",
            "Davis", "Rodriguez", "Martinez", "Hernandez", "Lopez", "Gonzalez",
            "Wilson", "Anderson", "Thomas", "Taylor", "Moore", "Jackson", "Martin",
            "Lee", "Perez", "Thompson", "White", "Harris", "Sanchez", "Clark",
            "Ramirez", "Lewis", "Robinson", "Walker", "Young", "Allen", "King",
            "Wright", "Scott", "Torres", "Nguyen", "Hill", "Flores", "Green",
            "Adams", "Nelson", "Baker", "Hall", "Rivera", "Campbell", "Mitchell",
            "Carter", "Roberts", "Turner", "Phillips", "Evans", "Edwards", "Collins",
            "Stewart", "Morris", "Rogers", "Cooper", "Reed", "Bailey", "Bell",
            "Murphy", "Cook", "Richardson", "Wood", "Watson", "Brooks", "Bennett"
        ]

    def generate_unique_name(self, existing_names: Set[str]) -> str:
        """生成唯一姓名"""
        attempts = 0
        while attempts < 100:  # 防止无限循环
            first = random.choice(self.first_names)
            last = random.choice(self.last_names)
            name = f"{first}_{last}"
            
            # 有时添加数字或中间名增加多样性
            if random.random() < 0.3:
                name += str(random.randint(1, 999))
            elif random.random() < 0.2:
                middle_initial = random.choice(string.ascii_uppercase)
                name = f"{first}_{middle_initial}_{last}"
            
            if name not in existing_names:
                return name
            attempts += 1
        
        # 如果生成不了唯一姓名，添加时间戳
        timestamp = datetime.now().strftime("%H%M%S")
        return f"{random.choice(self.first_names)}_{timestamp}"

    def generate_persons(self, count: int) -> List[Dict]:
        """生成人员列表"""
        existing_names = set()
        persons = []
        
        for i in range(count):
            name = self.generate_unique_name(existing_names)
            existing_names.add(name)
            
            # 添加一些属性增加多样性（可选）
            person = {"name": name}
            
            # 30%的概率添加额外属性
            if random.random() < 0.3:
                person["id"] = i + 1
            
            persons.append(person)
        
        return persons

    def generate_random_network(self, persons: List[Dict], connection_prob: float, 
                               weight_min: int, weight_max: int) -> Tuple[List[Dict], int]:
        """生成完全随机网络"""
        edges = []
        n = len(persons)
        edges_set = set()
        
        # 随机添加连接
        for i in range(n):
            for j in range(i + 1, n):
                if random.random() < connection_prob:
                    weight = random.randint(weight_min, weight_max)
                    edges.append({
                        "from": persons[i]["name"],
                        "to": persons[j]["name"],
                        "weight": weight
                    })
                    edges_set.add((i, j))
        
        # 统计孤立节点数量
        isolated_count = 0
        for i in range(n):
            has_connection = False
            for edge in edges:
                if persons[i]["name"] == edge["from"] or persons[i]["name"] == edge["to"]:
                    has_connection = True
                    break
            if not has_connection:
                isolated_count += 1
        
        return edges, isolated_count

    def generate_scale_free_network(self, persons: List[Dict], m: int, 
                                   weight_min: int, weight_max: int) -> Tuple[List[Dict], int]:
        """生成无标度网络（少数节点有很多连接）"""
        edges = []
        n = len(persons)
        
        # 初始连接
        for i in range(m):
            for j in range(i + 1, m):
                weight = random.randint(weight_min, weight_max)
                edges.append({
                    "from": persons[i]["name"],
                    "to": persons[j]["name"],
                    "weight": weight
                })
        
        # 偏好连接：新节点倾向于连接已经有很多连接的节点
        for i in range(m, n):
            # 计算每个节点的连接数作为概率权重
            degrees = [0] * i
            for edge in edges:
                for j in range(i):
                    if persons[j]["name"] == edge["from"] or persons[j]["name"] == edge["to"]:
                        degrees[j] += 1
            
            # 避免除零错误
            total_degree = sum(degrees) if sum(degrees) > 0 else 1
            
            # 新节点连接 m 个现有节点
            connections_made = 0
            attempts = 0
            
            while connections_made < m and attempts < n * 2:
                # 按度数比例选择节点
                r = random.random() * total_degree
                cumsum = 0
                selected = 0
                for j in range(i):
                    cumsum += degrees[j]
                    if cumsum >= r:
                        selected = j
                        break
                
                # 检查是否已连接
                already_connected = False
                for edge in edges:
                    if (persons[i]["name"] == edge["from"] and persons[selected]["name"] == edge["to"]) or \
                       (persons[i]["name"] == edge["to"] and persons[selected]["name"] == edge["from"]):
                        already_connected = True
                        break
                
                if not already_connected:
                    weight = random.randint(weight_min, weight_max)
                    edges.append({
                        "from": persons[i]["name"],
                        "to": persons[selected]["name"],
                        "weight": weight
                    })
                    degrees[selected] += 1
                    total_degree += 1
                    connections_made += 1
                
                attempts += 1
        
        # 计算孤立节点数量
        isolated_count = 0
        connected = [False] * n
        for edge in edges:
            from_idx = next(i for i, p in enumerate(persons) if p["name"] == edge["from"])
            to_idx = next(i for i, p in enumerate(persons) if p["name"] == edge["to"])
            connected[from_idx] = True
            connected[to_idx] = True
        
        isolated_count = sum(1 for c in connected if not c)
        return edges, isolated_count

    def generate_community_network(self, persons: List[Dict], num_communities: int,
                                  intra_prob: float, inter_prob: float,
                                  weight_min: int, weight_max: int) -> Tuple[List[Dict], int]:
        """生成社区结构网络"""
        edges = []
        n = len(persons)
        
        # 分配人员到社区
        community_size = n // num_communities
        communities = []
        start = 0
        for _ in range(num_communities - 1):
            end = start + community_size
            communities.append(list(range(start, end)))
            start = end
        communities.append(list(range(start, n)))  # 最后一个社区包含剩余人员
        
        # 生成社区内连接（高概率）
        for community in communities:
            for i_idx, i in enumerate(community):
                for j in community[i_idx + 1:]:
                    if random.random() < intra_prob:
                        weight = random.randint(weight_min, weight_max)
                        edges.append({
                            "from": persons[i]["name"],
                            "to": persons[j]["name"],
                            "weight": weight
                        })
        
        # 生成社区间连接（低概率）
        for ci in range(num_communities):
            for cj in range(ci + 1, num_communities):
                # 社区间随机连接几对节点
                num_bridges = max(1, int(min(len(communities[ci]), len(communities[cj])) * inter_prob))
                for _ in range(num_bridges):
                    if random.random() < inter_prob:
                        i = random.choice(communities[ci])
                        j = random.choice(communities[cj])
                        
                        # 检查是否已连接
                        already_connected = False
                        for edge in edges:
                            if (persons[i]["name"] == edge["from"] and persons[j]["name"] == edge["to"]) or \
                               (persons[i]["name"] == edge["to"] and persons[j]["name"] == edge["from"]):
                                already_connected = True
                                break
                        
                        if not already_connected:
                            weight = random.randint(weight_min // 2, weight_max)  # 社区间连接可能更弱
                            edges.append({
                                "from": persons[i]["name"],
                                "to": persons[j]["name"],
                                "weight": weight
                            })
        
        # 计算孤立节点数量
        connected = [False] * n
        for edge in edges:
            from_idx = next(i for i, p in enumerate(persons) if p["name"] == edge["from"])
            to_idx = next(i for i, p in enumerate(persons) if p["name"] == edge["to"])
            connected[from_idx] = True
            connected[to_idx] = True
        
        isolated_count = sum(1 for c in connected if not c)
        return edges, isolated_count

    def generate_star_network(self, persons: List[Dict], num_centers: int,
                             weight_min: int, weight_max: int) -> Tuple[List[Dict], int]:
        """生成星型网络"""
        edges = []
        n = len(persons)
        
        if n <= num_centers:
            return edges, n  # 所有人都是孤立的
        
        # 随机选择中心节点
        centers = random.sample(range(n), num_centers)
        peripherals = [i for i in range(n) if i not in centers]
        
        # 每个外围节点连接到一个中心节点
        for peripheral in peripherals:
            center = random.choice(centers)
            weight = random.randint(weight_min, weight_max)
            edges.append({
                "from": persons[peripheral]["name"],
                "to": persons[center]["name"],
                "weight": weight
            })
        
        # 中心节点之间可能连接
        for i_idx, i in enumerate(centers):
            for j in centers[i_idx + 1:]:
                if random.random() < 0.5:  # 50%概率连接中心节点
                    weight = random.randint(weight_min, weight_max)
                    edges.append({
                        "from": persons[i]["name"],
                        "to": persons[j]["name"],
                        "weight": weight
                    })
        
        # 计算孤立节点数量（理论上没有，除非n <= num_centers）
        connected = [False] * n
        for edge in edges:
            from_idx = next(i for i, p in enumerate(persons) if p["name"] == edge["from"])
            to_idx = next(i for i, p in enumerate(persons) if p["name"] == edge["to"])
            connected[from_idx] = True
            connected[to_idx] = True
        
        isolated_count = sum(1 for c in connected if not c)
        return edges, isolated_count

    def generate_hybrid_network(self, persons: List[Dict], weight_min: int, weight_max: int) -> Tuple[List[Dict], int]:
        """生成混合类型网络"""
        n = len(persons)
        
        # 随机决定网络结构
        structure_type = random.choice(["random_communities", "scale_free_with_isolates", "mixed"])
        
        if structure_type == "random_communities":
            # 随机社区 + 一些孤立节点
            num_communities = random.randint(2, max(2, n // 10))
            intra_prob = random.uniform(0.3, 0.8)
            inter_prob = random.uniform(0.01, 0.2)
            edges, isolated_count = self.generate_community_network(
                persons, num_communities, intra_prob, inter_prob, weight_min, weight_max
            )
            
            # 添加一些随机连接
            extra_edges = random.randint(0, n)
            for _ in range(extra_edges):
                i = random.randint(0, n - 1)
                j = random.randint(0, n - 1)
                if i != j:
                    already_connected = any(
                        (persons[i]["name"] == e["from"] and persons[j]["name"] == e["to"]) or
                        (persons[i]["name"] == e["to"] and persons[j]["name"] == e["from"])
                        for e in edges
                    )
                    if not already_connected and random.random() < 0.1:
                        weight = random.randint(weight_min, weight_max)
                        edges.append({
                            "from": persons[i]["name"],
                            "to": persons[j]["name"],
                            "weight": weight
                        })
        
        elif structure_type == "scale_free_with_isolates":
            # 无标度网络 + 故意添加孤立节点
            m = random.randint(1, min(5, n // 4))
            edges, isolated_count = self.generate_scale_free_network(persons, m, weight_min, weight_max)
            
            # 确保有一定比例的孤立节点
            target_isolated_ratio = random.uniform(0.05, 0.3)  # 5%-30%的孤立节点
            target_isolated = int(n * target_isolated_ratio)
            
            if isolated_count < target_isolated:
                # 随机断开一些连接以创建更多孤立节点
                connected_indices = []
                for i in range(n):
                    if any(persons[i]["name"] in [e["from"], e["to"]] for e in edges):
                        connected_indices.append(i)
                
                if connected_indices:
                    to_disconnect = random.sample(
                        connected_indices,
                        min(len(connected_indices), target_isolated - isolated_count)
                    )
                    # 移除这些节点的所有连接
                    edges = [
                        e for e in edges
                        if not any(persons[i]["name"] in [e["from"], e["to"]] for i in to_disconnect)
                    ]
                    isolated_count += len(to_disconnect)
        
        else:  # mixed
            # 混合多种结构
            edges = []
            
            # 1. 创建一个核心的无标度网络（60%的人）
            core_size = int(n * 0.6)
            if core_size > 2:
                core_indices = random.sample(range(n), core_size)
                core_persons = [persons[i] for i in core_indices]
                core_edges, _ = self.generate_scale_free_network(
                    core_persons, 
                    m=random.randint(1, min(3, core_size // 3)),
                    weight_min=weight_min,
                    weight_max=weight_max
                )
                edges.extend(core_edges)
            
            # 2. 剩余40%的人中，一半随机连接，一半孤立
            remaining_indices = [i for i in range(n) if i not in core_indices]
            num_to_connect = len(remaining_indices) // 2
            
            if num_to_connect > 0:
                # 随机连接一些人
                for _ in range(random.randint(0, num_to_connect * 2)):
                    i = random.choice(remaining_indices[:num_to_connect])
                    j = random.choice(list(range(n)))
                    if i != j:
                        already_connected = any(
                            (persons[i]["name"] == e["from"] and persons[j]["name"] == e["to"]) or
                            (persons[i]["name"] == e["to"] and persons[j]["name"] == e["from"])
                            for e in edges
                        )
                        if not already_connected and random.random() < 0.3:
                            weight = random.randint(weight_min, weight_max)
                            edges.append({
                                "from": persons[i]["name"],
                                "to": persons[j]["name"],
                                "weight": weight
                            })
            
            # 计算孤立节点数量
            connected = [False] * n
            for edge in edges:
                from_idx = next(i for i, p in enumerate(persons) if p["name"] == edge["from"])
                to_idx = next(i for i, p in enumerate(persons) if p["name"] == edge["to"])
                connected[from_idx] = True
                connected[to_idx] = True
            
            isolated_count = sum(1 for c in connected if not c)
        
        return edges, isolated_count

    def generate_network(self, network_type: str, person_count: int, 
                        weight_min: int = 1, weight_max: int = 100,
                        **kwargs) -> Dict:
        """
        生成指定类型的社交网络
        """
        # 生成人员
        persons = self.generate_persons(person_count)
        
        if network_type == "random":
            connection_prob = kwargs.get("connection_prob", 0.1)
            edges, isolated_count = self.generate_random_network(
                persons, connection_prob, weight_min, weight_max
            )
        
        elif network_type == "scale_free":
            m = kwargs.get("m", 2)  # 初始连接数
            edges, isolated_count = self.generate_scale_free_network(
                persons, m, weight_min, weight_max
            )
        
        elif network_type == "community":
            num_communities = kwargs.get("num_communities", 3)
            intra_prob = kwargs.get("intra_prob", 0.6)
            inter_prob = kwargs.get("inter_prob", 0.05)
            edges, isolated_count = self.generate_community_network(
                persons, num_communities, intra_prob, inter_prob, weight_min, weight_max
            )
        
        elif network_type == "star":
            num_centers = kwargs.get("num_centers", 1)
            edges, isolated_count = self.generate_star_network(
                persons, num_centers, weight_min, weight_max
            )
        
        elif network_type == "hybrid":
            edges, isolated_count = self.generate_hybrid_network(
                persons, weight_min, weight_max
            )
        
        else:
            raise ValueError(f"未知的网络类型: {network_type}")
        
        # 构建最终JSON
        network = {
            "persons": persons,
            "edges": edges,
            "metadata": {
                "person_count": len(persons),
                "edge_count": len(edges),
                "density": len(edges) * 2.0 / (len(persons) * (len(persons) - 1)) if len(persons) > 1 else 0,
                "isolated_count": isolated_count,
                "isolated_percentage": isolated_count / len(persons) * 100 if len(persons) > 0 else 0,
                "save_time": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
                "version": "2.0",
                "generator": "EnhancedNetworkGenerator",
                "network_type": network_type,
                "parameters": {
                    "person_count": person_count,
                    "weight_range": f"{weight_min}-{weight_max}",
                    **kwargs
                }
            }
        }
        
        return network

    def save_to_file(self, network: Dict, filename: str):
        """保存网络到文件"""
        with open(filename, 'w', encoding='utf-8') as f:
            json.dump(network, f, ensure_ascii=False, indent=4)
        
        metadata = network["metadata"]
        print(f"\n✓ 网络已保存到 {filename}")
        print(f"  网络类型: {metadata['network_type']}")
        print(f"  统计: {metadata['person_count']} 人, {metadata['edge_count']} 条关系")
        print(f"  密度: {metadata['density']:.4f}")
        print(f"  孤立节点: {metadata['isolated_count']} 个 ({metadata['isolated_percentage']:.1f}%)")
        if metadata['edge_count'] > 0:
            avg_weight = sum(e["weight"] for e in network["edges"]) / metadata["edge_count"]
            print(f"  平均亲密度: {avg_weight:.1f}")


def generate_example_networks():
    """生成一系列示例网络，展示不同结构"""
    generator = EnhancedNetworkGenerator(seed=42)
    
    examples = [
        {
            "name": "random_sparse.json",
            "type": "random",
            "person_count": 30,
            "params": {"connection_prob": 0.05}
        },
        {
            "name": "scale_free.json",
            "type": "scale_free",
            "person_count": 40,
            "params": {"m": 2}
        },
        {
            "name": "communities.json",
            "type": "community",
            "person_count": 50,
            "params": {"num_communities": 4, "intra_prob": 0.4, "inter_prob": 0.02}
        },
        {
            "name": "star_network.json",
            "type": "star",
            "person_count": 25,
            "params": {"num_centers": 2}
        },
        {
            "name": "hybrid_complex.json",
            "type": "hybrid",
            "person_count": 60,
            "params": {}
        },
        {
            "name": "ultra_sparse.json",
            "type": "random",
            "person_count": 20,
            "params": {"connection_prob": 0.02}
        },
        {
            "name": "dense_community.json",
            "type": "community",
            "person_count": 35,
            "params": {"num_communities": 2, "intra_prob": 0.8, "inter_prob": 0.1}
        }
    ]
    
    print("生成示例网络集:")
    print("=" * 60)
    
    for example in examples:
        network = generator.generate_network(
            network_type=example["type"],
            person_count=example["person_count"],
            weight_min=10,
            weight_max=95,
            **example["params"]
        )
        generator.save_to_file(network, example["name"])
    
    print("\n所有示例网络已生成完成！")


def interactive_mode():
    """交互式模式"""
    print("=" * 60)
    print("增强版社交网络生成器")
    print("支持多种网络结构，允许孤立节点")
    print("=" * 60)
    
    generator = EnhancedNetworkGenerator()
    
    while True:
        print("\n请选择网络类型:")
        print("1. 随机网络 (稀疏/密集)")
        print("2. 无标度网络 (少数中心节点)")
        print("3. 社区网络 (多个紧密小组)")
        print("4. 星型网络 (中心化结构)")
        print("5. 混合网络 (复杂结构)")
        print("6. 生成所有示例网络")
        print("7. 批量随机生成")
        print("0. 退出")
        
        choice = input("\n请选择 (0-7): ").strip()
        
        if choice == "0":
            print("再见！")
            break
        
        elif choice == "1":
            # 随机网络
            try:
                person_count = int(input("人数 (5-200): ").strip() or "30")
                connection_prob = float(input("连接概率 (0.01-0.5): ").strip() or "0.1")
                filename = input("文件名 (默认: random_network.json): ").strip() or "random_network.json"
                
                network = generator.generate_network(
                    network_type="random",
                    person_count=person_count,
                    connection_prob=connection_prob
                )
                generator.save_to_file(network, filename)
            except ValueError as e:
                print(f"输入错误: {e}")
        
        elif choice == "2":
            # 无标度网络
            try:
                person_count = int(input("人数 (10-150): ").strip() or "40")
                m = int(input("初始连接数 (1-10): ").strip() or "2")
                filename = input("文件名 (默认: scale_free.json): ").strip() or "scale_free.json"
                
                network = generator.generate_network(
                    network_type="scale_free",
                    person_count=person_count,
                    m=m
                )
                generator.save_to_file(network, filename)
            except ValueError as e:
                print(f"输入错误: {e}")
        
        elif choice == "3":
            # 社区网络
            try:
                person_count = int(input("人数 (15-100): ").strip() or "50")
                num_communities = int(input("社区数量 (2-10): ").strip() or "3")
                intra_prob = float(input("社区内连接概率 (0.2-0.9): ").strip() or "0.6")
                inter_prob = float(input("社区间连接概率 (0.01-0.3): ").strip() or "0.05")
                filename = input("文件名 (默认: communities.json): ").strip() or "communities.json"
                
                network = generator.generate_network(
                    network_type="community",
                    person_count=person_count,
                    num_communities=num_communities,
                    intra_prob=intra_prob,
                    inter_prob=inter_prob
                )
                generator.save_to_file(network, filename)
            except ValueError as e:
                print(f"输入错误: {e}")
        
        elif choice == "4":
            # 星型网络
            try:
                person_count = int(input("人数 (5-50): ").strip() or "25")
                num_centers = int(input("中心节点数量 (1-5): ").strip() or "1")
                filename = input("文件名 (默认: star_network.json): ").strip() or "star_network.json"
                
                network = generator.generate_network(
                    network_type="star",
                    person_count=person_count,
                    num_centers=num_centers
                )
                generator.save_to_file(network, filename)
            except ValueError as e:
                print(f"输入错误: {e}")
        
        elif choice == "5":
            # 混合网络
            try:
                person_count = int(input("人数 (20-200): ").strip() or "60")
                filename = input("文件名 (默认: hybrid_network.json): ").strip() or "hybrid_network.json"
                
                network = generator.generate_network(
                    network_type="hybrid",
                    person_count=person_count
                )
                generator.save_to_file(network, filename)
            except ValueError as e:
                print(f"输入错误: {e}")
        
        elif choice == "6":
            # 生成所有示例
            generate_example_networks()
        
        elif choice == "7":
            # 批量随机生成
            try:
                count = int(input("生成数量 (1-20): ").strip() or "5")
                min_persons = int(input("最小人数 (5-100): ").strip() or "10")
                max_persons = int(input("最大人数 (10-200): ").strip() or "50")
                base_name = input("基础文件名 (例如: network): ").strip() or "random_batch"
                
                for i in range(1, count + 1):
                    person_count = random.randint(min_persons, max_persons)
                    network_type = random.choice(["random", "scale_free", "community", "hybrid"])
                    
                    if network_type == "random":
                        params = {"connection_prob": random.uniform(0.02, 0.3)}
                    elif network_type == "scale_free":
                        params = {"m": random.randint(1, min(5, person_count // 10))}
                    elif network_type == "community":
                        params = {
                            "num_communities": random.randint(2, max(2, person_count // 15)),
                            "intra_prob": random.uniform(0.3, 0.8),
                            "inter_prob": random.uniform(0.01, 0.2)
                        }
                    else:  # hybrid
                        params = {}
                    
                    filename = f"{base_name}_{i:03d}.json"
                    network = generator.generate_network(
                        network_type=network_type,
                        person_count=person_count,
                        **params
                    )
                    generator.save_to_file(network, filename)
                
                print(f"\n✓ 批量生成完成！共生成 {count} 个网络文件。")
            
            except ValueError as e:
                print(f"输入错误: {e}")
        
        else:
            print("无效选择，请重试！")
        
        input("\n按Enter键继续...")


def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="增强版社交网络生成器")
    parser.add_argument("--interactive", "-i", action="store_true",
                       help="进入交互式模式")
    parser.add_argument("--type", "-t", choices=["random", "scale_free", "community", "star", "hybrid"],
                       help="网络类型")
    parser.add_argument("--output", "-o", default="network.json",
                       help="输出文件名")
    parser.add_argument("--persons", "-p", type=int, default=30,
                       help="人数")
    parser.add_argument("--examples", action="store_true",
                       help="生成示例网络集")
    parser.add_argument("--seed", type=int, help="随机种子")
    
    args = parser.parse_args()
    
    if args.examples:
        generate_example_networks()
    elif args.interactive or not args.type:
        interactive_mode()
    else:
        generator = EnhancedNetworkGenerator(seed=args.seed)
        
        # 根据类型设置默认参数
        params = {}
        if args.type == "random":
            params["connection_prob"] = 0.1
        elif args.type == "scale_free":
            params["m"] = 2
        elif args.type == "community":
            params["num_communities"] = 3
            params["intra_prob"] = 0.6
            params["inter_prob"] = 0.05
        elif args.type == "star":
            params["num_centers"] = 1
        
        network = generator.generate_network(
            network_type=args.type,
            person_count=args.persons,
            **params
        )
        generator.save_to_file(network, args.output)


if __name__ == "__main__":
    main()
